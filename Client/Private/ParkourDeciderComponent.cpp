#include "ClientPch.h"
#include "ParkourDeciderComponent.h"
#include "ParkourTuningTable.h"
#include "GameSystem.h"
#include "EnvironmentQueryComponent.h"
#include "InputController.h"
#include "Collider.h"
#include "StateMachine.h"
#include "TraceurState_Enum.h"
#include "StateCategory_Enum.h"
#include "Client_CharacterEnum.h"

CParkourDeciderComponent::CParkourDeciderComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CParkourDeciderComponent::CParkourDeciderComponent(const CParkourDeciderComponent& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CParkourDeciderComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParkourDeciderComponent::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	const PARKOUR_DECIDER_DESC* pDesc = static_cast<PARKOUR_DECIDER_DESC*>(pArg);
	if (nullptr == pDesc->pBodyProfile)
		return E_FAIL;

	m_pBodyProfile = pDesc->pBodyProfile;
	m_pTuning = CGameSystem::GetInstance()->Get_ParkourTuning();
	Safe_AddRef(m_pTuning);

	m_pInputCom = dynamic_cast<CInputController*>(m_pOwner->Get_Component(TEXT("Com_InputController")));
	if (nullptr == m_pInputCom) return E_FAIL;

	m_pColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pColliderCom) return E_FAIL;

	m_pStateMachineCom = dynamic_cast<CStateMachine*>(m_pOwner->Get_Component(TEXT("Com_StateMachine")));
	if (nullptr == m_pStateMachineCom) return E_FAIL;

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	if (nullptr == m_pEnvQueryCom) return E_FAIL;

	return S_OK;
}

void CParkourDeciderComponent::Decide(const ENV_PERCEPTION& Perception, _float fTimeDelta)
{
	m_Decision = {};
	Judge(Perception.Scan, Perception.Geometry);
	Update_Intent();
	Update_Context(fTimeDelta);
	Arbitrate();
}

void CParkourDeciderComponent::Update_Intent()
{
	const _uint iMoveKey = ENUM_CLASS(KEYINPUT::W) | ENUM_CLASS(KEYINPUT::A)
	                     | ENUM_CLASS(KEYINPUT::S) | ENUM_CLASS(KEYINPUT::D);
	m_Decision.hasMoveInput = m_pInputCom->Check_AnyInput(iMoveKey);
	m_Decision.wantsRun     = m_Decision.hasMoveInput && m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
	m_Decision.wantsJump    = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_Decision.wantsForward = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	m_Decision.wantsDown    = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	m_Decision.wantsLeft      = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	m_Decision.wantsRight     = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));
	m_Decision.wantsJumpPress = m_pInputCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE), Engine::KEYSTATE::DOWN);
}

void CParkourDeciderComponent::Request_ScanDir(_fvector vDir, const Engine::StateKey& Requester)
{
	m_hasScanDirRequest = true;
	m_ScanDirRequester  = Requester;
	XMStoreFloat3(&m_vRequestedScanDir, vDir);
	m_pEnvQueryCom->Set_ScanDirOverride(vDir);
}

void CParkourDeciderComponent::Clear_ScanDir()
{
	m_hasScanDirRequest = false;
	m_pEnvQueryCom->Clear_ScanDirOverride();
}

void CParkourDeciderComponent::Update_Context(_float fTimeDelta)
{
	const Engine::StateKey CurKey = m_pStateMachineCom->Get_CurrentStateKey();
	if (m_hasScanDirRequest && CurKey.iCategory != m_ScanDirRequester.iCategory)
		Clear_ScanDir();

	_float3 vGroundN{};
	m_Decision.isSupported = m_pColliderCom->IsLand(&vGroundN);
	m_Decision.isGrounded  = m_Decision.isSupported && vGroundN.y >= cosf(XMConvertToRadians(50.f));

	if (m_Decision.isSupported) m_fFallTime = 0.f;
	else                        m_fFallTime += fTimeDelta;
	m_Decision.isFalling = (m_fFallTime >= 0.3f);

	const _bool isGroundMove = (CurKey.iCategory == ENUM_CLASS(EStateCategory::GROUND)
	                         && CurKey.iSubState  == ENUM_CLASS(ETraceurGroundState::Move));
	const _bool wasGroundMove = (m_PrevStateKey.iCategory == CurKey.iCategory
	                          && m_PrevStateKey.iSubState  == CurKey.iSubState) && isGroundMove;
	if (isGroundMove && !wasGroundMove)
		m_fWallRunCooldown = m_pTuning->Get().fWallRunCooldown;
	else if (isGroundMove)
		m_fWallRunCooldown = max(m_fWallRunCooldown - fTimeDelta, 0.f);
	m_PrevStateKey = CurKey;
}

void CParkourDeciderComponent::Arbitrate()
{
	m_Decision.eCommand = PARKOUR_ACTION::NONE;
	const Engine::StateKey CurKey = m_pStateMachineCom->Get_CurrentStateKey();

	const _bool isHangPossible = m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::HANG)].isPossible;

	if (CurKey.iCategory == ENUM_CLASS(EStateCategory::AIR))
	{
		if (isHangPossible && m_Decision.wantsForward)
			m_Decision.eCommand = PARKOUR_ACTION::HANG;
		return;
	}

	if (CurKey.iCategory == ENUM_CLASS(EStateCategory::CLIMB))
	{
		if (CurKey.iSubState == ENUM_CLASS(ETraceurClimbState::Move) && isHangPossible)
			m_Decision.eCommand = PARKOUR_ACTION::HANG;
		return;
	}

	if (CurKey.iCategory != ENUM_CLASS(EStateCategory::GROUND)
	 || CurKey.iSubState  != ENUM_CLASS(ETraceurGroundState::Move))
		return;
	if (!m_Decision.isValid && !isHangPossible)
		return;

	for (PARKOUR_ACTION eAction : m_pTuning->Get().Priority)
	{
		switch (eAction)
		{
		case PARKOUR_ACTION::LOW_VAULT:
			if (m_Decision.eBestEnvAction == PARKOUR_ACTION::LOW_VAULT
			 && m_Decision.wantsRun && m_Decision.isGrounded)
				{ m_Decision.eCommand = eAction; return; }
			break;
		case PARKOUR_ACTION::WALL_RUN:
			if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::WALL_RUN)].isPossible
			 && m_Decision.wantsRun && m_fWallRunCooldown <= 0.f)
				{ m_Decision.eCommand = eAction; return; }
			break;
		case PARKOUR_ACTION::CLIMB:
			if (m_Decision.eBestEnvAction == PARKOUR_ACTION::CLIMB
			 && m_Decision.wantsForward && !m_Decision.wantsRun)
				{ m_Decision.eCommand = eAction; return; }
			break;
		case PARKOUR_ACTION::HANG:
			if (isHangPossible)
				{ m_Decision.eCommand = eAction; return; }
			break;
		default: break;
		}
	}
}

void CParkourDeciderComponent::Judge(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo)
{
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::HANG)] = Judge_Hang(Scan, Geo);

	if (Geo.Top.fHeight <= 0.f) return;

	_vector vLook = XMLoadFloat3(&Scan.vScanDir);
	_vector vApproachDir = XMVector3Normalize(XMVectorSetY(vLook, 0.f));
	m_Decision.fApproachDot = XMVectorGetX(XMVector3Dot(vApproachDir, XMLoadFloat3(&Geo.vTraversalDir)));

	const ACTION_VERDICT VaultVerdict = Judge_Vault(Scan, Geo, m_Decision.fApproachDot);

	_float fTotalHeight = m_pBodyProfile->fHeight;
	const _bool isHighVault = Geo.Top.fHeight >= fTotalHeight * m_pTuning->Get().Vault.fHighVaultHeightRatio;
	const ACTION_VERDICT NoMatch{ false, REJECT_REASON::NO_HEIGHT_MATCH };
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::LOW_VAULT)]  = isHighVault ? NoMatch : VaultVerdict;
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::HIGH_VAULT)] = isHighVault ? VaultVerdict : NoMatch;
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::MANTLE)] = Judge_Mantle(Scan, Geo, m_Decision.fApproachDot);
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::CLIMB)]  = Judge_Climb(Scan, Geo);
	m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::WALL_RUN)] = Judge_WallRun(Scan, Geo, m_Decision.fApproachDot);

	m_Decision.iCandidateFlag = 0;
	if (VaultVerdict.isPossible)
		m_Decision.iCandidateFlag |= ENUM_CLASS(PARKOUR_FLAG::VAULTABLE);
	if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::MANTLE)].isPossible)
		m_Decision.iCandidateFlag |= ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE);
	if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::CLIMB)].isPossible)
		m_Decision.iCandidateFlag |= ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE);

	if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::CLIMB)].isPossible)
	{
		m_Decision.eBestEnvAction = PARKOUR_ACTION::CLIMB;
		m_Decision.isValid = true;
	}
	else if (VaultVerdict.isPossible)
	{
		m_Decision.eBestEnvAction = isHighVault ? PARKOUR_ACTION::HIGH_VAULT : PARKOUR_ACTION::LOW_VAULT;
		m_Decision.isValid = true;
	}
	else if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::MANTLE)].isPossible)
	{
		m_Decision.eBestEnvAction = PARKOUR_ACTION::MANTLE;
		m_Decision.isValid = true;
	}

	if (m_Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::WALL_RUN)].isPossible)
	{
		m_Decision.iCandidateFlag |= ENUM_CLASS(PARKOUR_FLAG::WALLRUNNABLE);
		m_Decision.isValid = true;
	}
}

_uint CParkourDeciderComponent::Get_ObjectFlagMask(const OBSTACLE_SCAN& Scan) const
{
	_uint iMask = ENUM_CLASS(Scan.eObjectFlag);
	if (iMask >= ENUM_CLASS(PARKOUR_FLAG::END))
		iMask = ENUM_CLASS(PARKOUR_FLAG::ALL);
	else if (iMask == 0xF)
		iMask = ENUM_CLASS(PARKOUR_FLAG::ALL);
	return iMask;
}

ACTION_VERDICT CParkourDeciderComponent::Judge_Vault(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const
{
	const _bool bKnee = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE)) != 0;
	const _bool bHead = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD)) != 0;
	if (!bKnee || bHead)
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	if (!(Get_ObjectFlagMask(Scan) & ENUM_CLASS(PARKOUR_FLAG::VAULTABLE)))
		return { false, REJECT_REASON::FLAG_DENIED };

	if (!Geo.Top.isReachable)
		return { false, REJECT_REASON::TOP_UNREACHABLE };

	if (!Geo.Landing.hasSpace)
		return { false, REJECT_REASON::NO_LANDING };

	if (fApproachDot <= m_pTuning->Get().Vault.fMinApproachDot)
		return { false, REJECT_REASON::BAD_ANGLE };

	return { true, REJECT_REASON::NONE };
}

ACTION_VERDICT CParkourDeciderComponent::Judge_Mantle(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const
{
	const _bool bKnee = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE)) != 0;
	const _bool bHead = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD)) != 0;
	if (!bKnee || bHead)
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	if (!(Get_ObjectFlagMask(Scan) & ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE)))
		return { false, REJECT_REASON::FLAG_DENIED };

	if (!Geo.Top.isReachable)
		return { false, REJECT_REASON::TOP_UNREACHABLE };

	_float fRadius = m_pBodyProfile->fRadius;
	if (Geo.fDepth < fRadius * m_pTuning->Get().Mantle.fMinDepthMult)
		return { false, REJECT_REASON::TOO_THIN };

	if (Geo.fTopWidth < fRadius * m_pTuning->Get().Mantle.fMinWidthMult)
		return { false, REJECT_REASON::TOO_NARROW };

	if (fApproachDot <= m_pTuning->Get().Mantle.fMinApproachDot)
		return { false, REJECT_REASON::BAD_ANGLE };

	return { true, REJECT_REASON::NONE };
}

ACTION_VERDICT CParkourDeciderComponent::Judge_Climb(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const
{
	const _bool bKnee  = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE))  != 0;
	const _bool bChest = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST)) != 0;
	const _bool bHead  = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD))  != 0;
	if (!(bKnee && bChest && bHead))
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	if (!(Get_ObjectFlagMask(Scan) & ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE)))
		return { false, REJECT_REASON::FLAG_DENIED };

	_float fTotalHeight = m_pBodyProfile->fHeight;
	if (Geo.Top.isReachable && Geo.Top.fHeight > fTotalHeight * m_pTuning->Get().Climb.fMaxHeightRatio)
		return { false, REJECT_REASON::TOO_HIGH };

	return { true, REJECT_REASON::NONE };
}

ACTION_VERDICT CParkourDeciderComponent::Judge_Hang(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const
{
	(void)Geo;
	if (!Scan.Reach.Hit.isHit || !Scan.Reach.hasEdge)
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	// HANGABLE 명시 필수 — 무태그(END)의 ALL 폴백 불인정. 모든 벽에서 매달리는 것을 방지.
	const _uint iFlag = ENUM_CLASS(Scan.Reach.eObjectFlag);
	if (iFlag >= ENUM_CLASS(PARKOUR_FLAG::END) || !(iFlag & ENUM_CLASS(PARKOUR_FLAG::HANGABLE)))
		return { false, REJECT_REASON::FLAG_DENIED };

	const HANG_TUNING& T = m_pTuning->Get().Hang;
	const _float fH = m_pBodyProfile->fHeight;
	if (Scan.Reach.fEdgeHeight < fH * T.fMinTopHeightMult
	 || Scan.Reach.fEdgeHeight > fH * T.fMaxTopHeightMult)
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	if (fabsf(Scan.Reach.Hit.vHitNormal.y) > T.fMaxNormalY)
		return { false, REJECT_REASON::NOT_VERTICAL };

	_vector vApproach = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&Scan.vScanDir), 0.f));
	_vector vFace     = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&Scan.Reach.Hit.vHitNormal), 0.f));
	if (XMVectorGetX(XMVector3Dot(vApproach, XMVectorNegate(vFace))) <= T.fMinApproachDot)
		return { false, REJECT_REASON::BAD_ANGLE };

	return { true, REJECT_REASON::NONE };
}

ACTION_VERDICT CParkourDeciderComponent::Judge_WallRun(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const
{
	const _bool bKnee  = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE))  != 0;
	const _bool bChest = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST)) != 0;
	const _bool bHead  = (Scan.iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD))  != 0;
	if (!(bKnee && bChest && bHead))
		return { false, REJECT_REASON::NO_HEIGHT_MATCH };

	if (!(Get_ObjectFlagMask(Scan) & ENUM_CLASS(PARKOUR_FLAG::WALLRUNNABLE)))
		return { false, REJECT_REASON::FLAG_DENIED };

	if (!Geo.Front.hasHit || fabsf(Geo.Front.vNormal.y) > m_pTuning->Get().WallRun.fMaxNormalY)
		return { false, REJECT_REASON::NOT_VERTICAL };

	if (fApproachDot <= m_pTuning->Get().WallRun.fMinApproachDot)
		return { false, REJECT_REASON::BAD_ANGLE };

	if (Scan.ChestHit.fCenterDistance > m_pBodyProfile->fRadius * m_pTuning->Get().WallRun.fMaxStartDistMult)
		return { false, REJECT_REASON::TOO_FAR };

	return { true, REJECT_REASON::NONE };
}

#ifdef _DEBUG
void CParkourDeciderComponent::Print_Debug(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo)
{
	(void)Scan; (void)Geo;
}
#endif

CParkourDeciderComponent* CParkourDeciderComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParkourDeciderComponent* pInstance = new CParkourDeciderComponent(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CParkourDeciderComponent");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CParkourDeciderComponent::Clone(void* pArg)
{
	CParkourDeciderComponent* pInstance = new CParkourDeciderComponent(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CParkourDeciderComponent");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CParkourDeciderComponent::Free()
{
	__super::Free();
	Safe_Release(m_pTuning);
}

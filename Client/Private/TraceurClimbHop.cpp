#include "ClientPch.h"
#include "TraceurClimbHop.h"
#include "AnimationController.h"
#include "Model.h"
#include "Traceur.h"
#include "Collider.h"
#include "MotionWarpingComponent.h"
#include "EnvironmentQueryComponent.h"
#include "ParkourDeciderComponent.h"
#include "GameSystem.h"
#include "ParkourTuningTable.h"
#include "ParkourMath.h"

HRESULT CTraceurClimbHop::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurClimbHop::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);

	const ETraceurClimbHop eDir = static_cast<ETraceurClimbHop>(Get_CurrentAnim());

	HANG_TARGET Target{};
	if (!Find_HangTarget(eDir, Target))
	{
		Fallback(eDir);
		return;
	}

	HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	Ctx.isValid      = true;
	Ctx.vGrabEdgePos = Target.vGrabEdgePos;
	Ctx.vWallNormal  = Target.vWallNormal;
	Ctx.GrabBodyID   = Target.GrabBodyID;

	const HANG_TUNING&  T     = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	const BODY_PROFILE* pBody = m_pOwner->Get_BodyProfile();
	_vector vNormal = XMLoadFloat3(&Target.vWallNormal);

	_vector vP2 = ParkourMath::Calc_HangPos(XMLoadFloat3(&Target.vGrabEdgePos), vNormal,
		pBody->fRadius, pBody->fHeight, T.fHangOffsetMult, T.fWallOffset);

	_float3 vTargetPos{};
	XMStoreFloat3(&vTargetPos, vP2);
	m_pMotionWarpCom->Clear_WarpTargets();
	m_pMotionWarpCom->Set_WarpTarget("HopTarget", vTargetPos);
}

void CTraceurClimbHop::OnExit()
{
	__super::OnExit();
	m_pMotionWarpCom->End_RootWarp();
}

void CTraceurClimbHop::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(Engine::STATE::POSITION));

#ifdef _DEBUG
	Draw_DebugProbe();
#endif
}

_bool CTraceurClimbHop::Calc_ProbeSegment(const HANG_CONTEXT& Ctx, const HANG_TUNING& T,
	ETraceurClimbHop eDir, _vector& vOutStart, _vector& vOutDir, _float& fOutDist)
{
	if (!Ctx.isValid)
		return false;

	_float3 vMin{}, vMax{};
	if (!CGameInstance::GetInstance()->Get_Body_AABB(Ctx.GrabBodyID, vMin, vMax))
		return false;

	_vector vNormal = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&Ctx.vWallNormal), 0.f));
	_vector vUp     = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vRight  = XMVector3Normalize(XMVector3Cross(vUp, XMVectorNegate(vNormal)));

	_vector vAnchor = XMLoadFloat3(&Ctx.vGrabEdgePos) - vNormal * 0.05f;

	switch (eDir)
	{
	case ETraceurClimbHop::HopLeft:  vOutDir = XMVectorNegate(vRight); fOutDist = T.fHopDistLR;   break;
	case ETraceurClimbHop::HopRight: vOutDir = vRight;                 fOutDist = T.fHopDistLR;   break;
	case ETraceurClimbHop::HopUp:    vOutDir = vUp;                    fOutDist = T.fHopDistUp;   break;
	case ETraceurClimbHop::HopDrop:  vOutDir = XMVectorNegate(vUp);    fOutDist = T.fHopDistDown; break;
	default:
		return false;
	}

	// 진행 방향 쪽 현 바디 면까지의 거리
	_float fFaceDist = 0.f;
	for (_uint i = 0; i < 8; ++i)
	{
		_vector vCorner = XMVectorSet(
			(i & 1) ? vMax.x : vMin.x,
			(i & 2) ? vMax.y : vMin.y,
			(i & 4) ? vMax.z : vMin.z, 1.f);
		fFaceDist = max(fFaceDist, XMVectorGetX(XMVector3Dot(vCorner - vAnchor, vOutDir)));
	}
	vOutStart = vAnchor + vOutDir * (fFaceDist + T.fHopProbeRadius + 0.02f);
	return true;
}

#ifdef _DEBUG
void CTraceurClimbHop::Draw_DebugProbe() const
{
	CGameInstance* pGI = CGameInstance::GetInstance();
	if (!m_hasDebugProbe || !pGI->IsParkourDebug())
		return;

	pGI->Add_DebugLine(XMLoadFloat3(&m_vDebugProbeStart), XMLoadFloat3(&m_vDebugProbeEnd), JPH::Color(255, 140, 0, 255));
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vDebugProbeStart), 0.05f, JPH::Color(255, 140, 0, 255));
	if (m_isDebugProbeHit)
		pGI->Add_DebugSphere(XMLoadFloat3(&m_vDebugProbeHitPos), 0.08f, JPH::Color(255, 0, 0, 255));
}
#endif

void CTraceurClimbHop::Late_Anim_Update(_float fTimeDelta)
{
	// 이동은 Model의 루트모션 워프(Sync_RootNode)가 전담 — 상태는 구동할 것이 없다 (root warp)
	(void)fTimeDelta;
}

_bool CTraceurClimbHop::Find_HangTarget(ETraceurClimbHop eDir, HANG_TARGET& Out) const
{
	const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	if (!Ctx.isValid)
		return false;

	const HANG_TUNING& T = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	CGameInstance* pGI = CGameInstance::GetInstance();

	_vector vNormal = XMVector3Normalize(XMVectorSetY(XMLoadFloat3(&Ctx.vWallNormal), 0.f));

	_vector vDir;
	_vector vStart;
	_float  fDist = 0.f;
	if (!Calc_ProbeSegment(Ctx, T, eDir, vStart, vDir, fDist))
		return false;

#ifdef _DEBUG
	XMStoreFloat3(&m_vDebugProbeStart, vStart);
	XMStoreFloat3(&m_vDebugProbeEnd, vStart + vDir * fDist);
	m_hasDebugProbe   = true;
	m_isDebugProbeHit = false;
#endif

	SHAPE_CAST_HIT Hit{};
	const _bool isProbeHit = pGI->Sphere_Cast(vStart, vDir, fDist, T.fHopProbeRadius,
			ENUM_CLASS(COLLISIONLAYER::PARKOUR), Hit);
#ifdef _DEBUG
	if (isProbeHit)
	{
		m_isDebugProbeHit = true;
		m_vDebugProbeHitPos = _float3(Hit.vHitPoint.x, Hit.vHitPoint.y, Hit.vHitPoint.z);
	}
#endif
	if (!isProbeHit)
	{
#ifdef _DEBUG
		cout << "[Hop] reject: probe miss\n";
#endif
		return false;
	}

	_uint iFlag = ENUM_CLASS(PARKOUR_FLAG::END);
	if (Hit.pDesc)
		iFlag = ENUM_CLASS(static_cast<CALLBACK_CLIENT*>(Hit.pDesc)->eObjectParkourFlag);
	if (iFlag >= ENUM_CLASS(PARKOUR_FLAG::END) || !(iFlag & ENUM_CLASS(PARKOUR_FLAG::HANGABLE)))
	{
#ifdef _DEBUG
		cout << "[Hop] reject: flag denied (" << iFlag << ")\n";
#endif
		return false;
	}

	_vector vHitPos = XMVectorSet(Hit.vHitPoint.x, Hit.vHitPoint.y, Hit.vHitPoint.z, 1.f);

	// 히트 바디의 월드 AABB — 상단면 탐색 시작 높이 + 그랩 중앙 정렬 기준 (top face)
	_float3 vBodyMin{}, vBodyMax{};
	if (!pGI->Get_Body_AABB(Hit.HitBodyID, vBodyMin, vBodyMax))
	{
#ifdef _DEBUG
		cout << "[Hop] reject: body AABB query failed\n";
#endif
		return false;
	}

	// 그랩 기둥 = 타겟 바디의 가로 중앙 — 스윕 히트 지점의 측면 치우침 제거 (center)
	_vector vBodyCenter = XMVectorSet((vBodyMin.x + vBodyMax.x) * 0.5f, 0.f,
	                                  (vBodyMin.z + vBodyMax.z) * 0.5f, 1.f);

	RAY_CAST_HIT TopHit = pGI->Ray_Cast(
		XMVectorSetY(vBodyCenter, vBodyMax.y + 0.1f),
		XMVectorSetY(vBodyCenter, XMVectorGetY(vHitPos) - 0.2f),
		ENUM_CLASS(COLLISIONLAYER::PARKOUR));
	if (!TopHit.isHit)
	{
#ifdef _DEBUG
		cout << "[Hop] reject: top-face miss  hit=(" << Hit.vHitPoint.x << ", " << Hit.vHitPoint.y
		     << ", " << Hit.vHitPoint.z << ")  center=(" << XMVectorGetX(vBodyCenter) << ", " << XMVectorGetZ(vBodyCenter)
		     << ")  ray y " << (vBodyMax.y + 0.1f) << " -> " << (Hit.vHitPoint.y - 0.2f) << "\n";
#endif
		return false;
	}

	// 전면 레이: 중앙 기둥에서 바깥 -> 벽. 시작은 전면 지지거리 밖 보장 (front)
	const _float fFrontHalf = fabsf(XMVectorGetX(vNormal)) * (vBodyMax.x - vBodyMin.x) * 0.5f
	                        + fabsf(XMVectorGetZ(vNormal)) * (vBodyMax.z - vBodyMin.z) * 0.5f;
	const _float fFrontY = TopHit.vHitPosition.y - 0.08f;
	RAY_CAST_HIT FrontHit = pGI->Ray_Cast(
		XMVectorSetY(vBodyCenter + vNormal * (fFrontHalf + 0.3f), fFrontY),
		XMVectorSetY(vBodyCenter - vNormal * 0.2f, fFrontY),
		ENUM_CLASS(COLLISIONLAYER::PARKOUR));
	if (!FrontHit.isHit)
	{
#ifdef _DEBUG
		cout << "[Hop] reject: front-face miss  topY=" << TopHit.vHitPosition.y << "\n";
#endif
		return false;
	}

	_vector vTargetN = XMVectorSetY(XMLoadFloat3(&FrontHit.vHitNormal), 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vTargetN)) < 1e-4f)
		return false;
	vTargetN = XMVector3Normalize(vTargetN);
	if (XMVectorGetX(XMVector3Dot(vTargetN, vNormal)) < T.fMinNormalDot)
	{
#ifdef _DEBUG
		cout << "[Hop] reject: normal dot < " << T.fMinNormalDot << "\n";
#endif
		return false;
	}

	Out.vGrabEdgePos = _float3(FrontHit.vHitPosition.x, TopHit.vHitPosition.y, FrontHit.vHitPosition.z);
	XMStoreFloat3(&Out.vWallNormal, vTargetN);
	Out.GrabBodyID = Hit.HitBodyID;
	return true;
}

_bool CTraceurClimbHop::Can_StandTop(_float3& vOutStandPos) const
{
	const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	const HANG_TUNING&  T   = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	const BODY_PROFILE* pBody = m_pOwner->Get_BodyProfile();
	CGameInstance* pGI = CGameInstance::GetInstance();

	// 현 바디 AABB로 실제 상단 깊이 측정 — 지름 미만이면 설 수 없는 얇은 판 (depth)
	_float3 vMin{}, vMax{};
	if (!pGI->Get_Body_AABB(Ctx.GrabBodyID, vMin, vMax))
	{
#ifdef _DEBUG
		cout << "[Hop] standtop reject: body AABB query failed\n";
#endif
		return false;
	}

	_vector vInward = XMVectorNegate(XMLoadFloat3(&Ctx.vWallNormal));
	const _float fDepth = fabsf(XMVectorGetX(vInward)) * (vMax.x - vMin.x)
	                    + fabsf(XMVectorGetZ(vInward)) * (vMax.z - vMin.z);

	// 설 표면 후보: 판이 충분히 두꺼우면 판 자신의 상단, 얇으면 판 뒤 벽의 상단 (A안)
	// 프로브 XZ = 엣지 + 안쪽으로 (판 깊이 + 침투깊이) — 사용자 정의 지오메트리 기준 (probe)
	const _bool isThin = (fDepth < pBody->fRadius * 2.f);
	const _float fProbeInward = isThin
		? fDepth + T.fStandProbeInset
		: min(pBody->fRadius * T.fTopStandDepthMult, fDepth - 0.05f);
	_vector vProbe = XMLoadFloat3(&Ctx.vGrabEdgePos) + vInward * fProbeInward;

	// 다운캐스트: 잡은 바디 상단 + standProbeUp 에서 아래로 — 시작 높이도 튜닝 기준 (top)
	const _float fRayTopY = vMax.y + T.fStandProbeUp;
	const _float fRayBotY = Ctx.vGrabEdgePos.y - 0.3f;
	RAY_CAST_HIT TopHit = pGI->Ray_Cast(
		XMVectorSetY(vProbe, fRayTopY),
		XMVectorSetY(vProbe, fRayBotY),
		ENUM_CLASS(COLLISIONLAYER::PARKOUR));
	if (!TopHit.isHit)
		TopHit = pGI->Ray_Cast(
			XMVectorSetY(vProbe, fRayTopY),
			XMVectorSetY(vProbe, fRayBotY),
			ENUM_CLASS(COLLISIONLAYER::MAP));

	// 허용창: 엣지 -0.3 ~ 엣지 + standMaxRise (벽 상단이 엣지보다 조금 높아도 기립 허용)
	const _float fRise = TopHit.isHit ? (TopHit.vHitPosition.y - Ctx.vGrabEdgePos.y) : 0.f;
	if (!TopHit.isHit || fRise < -0.3f || fRise > T.fStandMaxRise)
	{
#ifdef _DEBUG
		cout << "[Hop] standtop reject: top-face " << (TopHit.isHit ? "out of window" : "miss")
		     << (isThin ? " (behind-wall probe)" : "")
		     << "  edgeY=" << Ctx.vGrabEdgePos.y
		     << "  rayY " << fRayTopY << " -> " << fRayBotY
		     << (TopHit.isHit ? ("  topY=" + to_string(TopHit.vHitPosition.y)) : string()) << "\n";
#endif
		return false;
	}

	_vector vUpStart = XMVectorSetY(vProbe, TopHit.vHitPosition.y + 0.1f);
	_vector vUpEnd   = XMVectorSetY(vProbe, TopHit.vHitPosition.y + pBody->fHeight);
	if (pGI->Ray_Cast(vUpStart, vUpEnd, ENUM_CLASS(COLLISIONLAYER::PARKOUR)).isHit)
	{
#ifdef _DEBUG
		cout << "[Hop] standtop reject: headroom blocked (PARKOUR)\n";
#endif
		return false;
	}
	if (pGI->Ray_Cast(vUpStart, vUpEnd, ENUM_CLASS(COLLISIONLAYER::MAP)).isHit)
	{
#ifdef _DEBUG
		cout << "[Hop] standtop reject: headroom blocked (MAP)\n";
#endif
		return false;
	}

	XMStoreFloat3(&vOutStandPos, XMVectorSetY(vProbe, TopHit.vHitPosition.y));
	return true;
}

void CTraceurClimbHop::Fallback(ETraceurClimbHop eDir)
{
	switch (eDir)
	{
	case ETraceurClimbHop::HopDrop:
	{
		// 아래 난간 없음 -> 손 놓고 낙하. 착지/낙하는 기존 CLIMB/Exit byAnim 규칙이 처리 (drop)
		m_pOwner->Get_HangContext().Reset();
		STATE_ENTER_DESC Desc{};
		Desc.iAnimIndex = ENUM_CLASS(ETraceurClimbExit::BracedHangDrop);
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Exit), &Desc);
		return;
	}
	case ETraceurClimbHop::HopUp:
	{
		_float3 vStandPos{};
		if (Can_StandTop(vStandPos))
		{
			const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
			STATE_ENTER_DESC Desc{};
			Desc.iAnimIndex     = ENUM_CLASS(ETraceurClimbExit::BracedHangToCrouch);
			Desc.hasEnvSnapshot = true;
			Desc.Perception     = m_pEnvQueryCom->Get_Perception();
			Desc.Decision       = m_pDeciderCom->Get_Decision();
			OBSTACLE_GEOMETRY& Geo = Desc.Perception.Geometry;
			Geo.Top.isReachable  = true;
			Geo.Top.vEdgePos     = _float3(Ctx.vGrabEdgePos.x, vStandPos.y, Ctx.vGrabEdgePos.z);
			Geo.Top.vStandPos    = vStandPos;
			Geo.Front.vNormal    = Ctx.vWallNormal;
			Geo.Landing.hasSpace = false;
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Exit), &Desc);
			return;
		}
		break;
	}
	default:
		break;
	}

	m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Hang));
}

CTraceurClimbHop* CTraceurClimbHop::Create(CTraceur* pOwner)
{
	CTraceurClimbHop* pInstance = new CTraceurClimbHop();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbHop");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbHop::Free()
{
	__super::Free();
}

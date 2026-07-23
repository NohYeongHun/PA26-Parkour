#include "ClientPch.h"
#include "TraceurClimbHang.h"
#include "AnimationController.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "TraceurClimbHop.h"
#include "MotionWarpingComponent.h"
#include "GameSystem.h"
#include "ParkourTuningTable.h"
#include "ParkourMath.h"
#include "IKDriver.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbHang::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurClimbHang::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_isHangIK = false;
	m_pColliderCom->Set_Gravity(false);
	Request_Anim(ENUM_CLASS(ETraceurClimbHang::HopIdle));


	if (!Ready_Hang(pArg))
	{
		m_pOwner->Get_HangContext().Reset();
		m_pColliderCom->Set_Gravity(true);
		const PARKOUR_DECISION& D = Enter_Decision(pArg);
		if (D.isGrounded)
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		else
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall));
		return;
	}

	// Ready_Hang 성공 — 손 앵커는 HANG_CONTEXT(진입 경로가 래치)에서 읽는다.
	m_isHangIK = true;
}

void CTraceurClimbHang::OnExit()
{
	__super::OnExit();
	m_pMotionWarpCom->End_CurveWarp();

	m_isHangIK = false;
}

void CTraceurClimbHang::Build_IKRequests(vector<IK_REQUEST>& Out)
{
	if (!m_isHangIK)
		return;

	const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	if (!Ctx.isValid)
		return;

	const HANG_TUNING& T = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;

	Out.push_back(IK_REQUEST::Fixed("LeftArmTwoBone",  XMLoadFloat3(&Ctx.vGrabL), XMLoadFloat3(&Ctx.vGrabN), 1.f, 1.f, 0.4f, 0.2f));
	Out.push_back(IK_REQUEST::Fixed("RightArmTwoBone", XMLoadFloat3(&Ctx.vGrabR), XMLoadFloat3(&Ctx.vGrabN), 1.f, 1.f, 0.4f, 0.2f));

	Out.push_back(IK_REQUEST::WallProbe("LeftLegTwoBone",  EIKTARGET_MODE::POSITION_CLEARANCE, XMLoadFloat3(&Ctx.vWallNormal),
		T.fFootPosWeight, T.fFootRotWeight, T.fFootBlendSec, 0.2f,
		T.fFootProbeOut, T.fFootProbeDepth, T.fFootSkin));
	Out.push_back(IK_REQUEST::WallProbe("RightLegTwoBone", EIKTARGET_MODE::POSITION_CLEARANCE, XMLoadFloat3(&Ctx.vWallNormal),
		T.fFootPosWeight, T.fFootRotWeight, T.fFootBlendSec, 0.2f,
		T.fFootProbeOut, T.fFootProbeDepth, T.fFootSkin));
}

_bool CTraceurClimbHang::Ready_Hang(void* pArg)
{
	HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	if (!Ctx.isValid)
	{
		// 공중 잡기 등 직접 진입 — 스냅샷의 ReachHit로 컨텍스트 구성 (air grab)
		const OBSTACLE_SCAN& Scan = Enter_Perception(pArg).Scan;
		if (!Scan.Reach.Hit.isHit || !Scan.Reach.hasEdge)
			return false;

		_vector vN = XMVectorSetY(XMLoadFloat3(&Scan.Reach.Hit.vHitNormal), 0.f);
		if (XMVectorGetX(XMVector3LengthSq(vN)) < 1e-4f)
			return false;

		Set_HangContext(XMLoadFloat3(&Scan.Reach.vEdgePos), XMVector3Normalize(vN), Scan.Reach.HitBodyID);
	}

	// 행 포즈로 짧은 시간 기반 커브워프 스냅 (snap)
	const HANG_TUNING&  T     = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	const BODY_PROFILE* pBody = m_pOwner->Get_BodyProfile();
	_vector vNormal = XMLoadFloat3(&Ctx.vWallNormal);

	_vector vP0 = m_pTransformCom->Get_State(Engine::STATE::POSITION);
	_vector vP2 = ParkourMath::Calc_HangPos(XMLoadFloat3(&Ctx.vGrabEdgePos), vNormal,
		pBody->fRadius, pBody->fHeight, T.fHangOffsetMult, T.fWallOffset);

	m_fSnapElapsed = 0.f;
	m_isSnapping   = true;
	m_pMotionWarpCom->Begin_CurveWarp(vP0, vP2, 0.f,
		m_pTransformCom->Get_State(Engine::STATE::LOOK), XMVectorNegate(vNormal));
	return true;
}

void CTraceurClimbHang::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(Engine::STATE::POSITION));

#ifdef _DEBUG
	Draw_DebugProbes();
#endif
}

#ifdef _DEBUG
void CTraceurClimbHang::Draw_DebugProbes() const
{
	CGameInstance* pGI = CGameInstance::GetInstance();
	if (!pGI->IsParkourDebug())
		return;

	const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	const HANG_TUNING&  T   = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;

	const struct { ETraceurClimbHop eDir; JPH::Color color; } Probes[] = {
		{ ETraceurClimbHop::HopLeft,  JPH::Color(255, 255,   0, 255) }, // 노랑 (L)
		{ ETraceurClimbHop::HopRight, JPH::Color(255,   0, 255, 255) }, // 마젠타 (R)
		{ ETraceurClimbHop::HopUp,    JPH::Color(  0, 255,   0, 255) }, // 초록 (Up)
		{ ETraceurClimbHop::HopDrop,  JPH::Color(  0, 128, 255, 255) }, // 하늘 (Drop)
	};

	for (const auto& P : Probes)
	{
		_vector vStart, vDir;
		_float  fDist = 0.f;
		if (!CTraceurClimbHop::Calc_ProbeSegment(Ctx, T, P.eDir, vStart, vDir, fDist))
			continue;
		pGI->Add_DebugLine(vStart, vStart + vDir * fDist, P.color);
		pGI->Add_DebugSphere(vStart, 0.05f, P.color);
	}
}
#endif

void CTraceurClimbHang::Late_Anim_Update(_float fTimeDelta)
{
	if (!m_isSnapping)
		return;

	const HANG_TUNING& T = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	m_fSnapElapsed += fTimeDelta;
	const _float fT = (T.fSnapTime <= 0.f) ? 1.f : min(m_fSnapElapsed / T.fSnapTime, 1.f);
	m_pMotionWarpCom->Update_CurveWarp(fT);
	if (fT >= 1.f)
	{
		m_isSnapping = false;
		m_pMotionWarpCom->End_CurveWarp();
	}
}

CTraceurClimbHang* CTraceurClimbHang::Create(CTraceur* pOwner)
{
	CTraceurClimbHang* pInstance = new CTraceurClimbHang();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbHang");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbHang::Free()
{
	__super::Free();
}

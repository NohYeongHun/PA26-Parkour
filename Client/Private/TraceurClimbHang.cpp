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

	// 1. Ready_Hang이 성공했다면? IK 실행. => Hang은 계속 유지되어야함.
	HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();
	_vector vEdge = XMLoadFloat3(&Ctx.vGrabEdgePos);
	_vector vTrav = XMVectorNegate(XMLoadFloat3(&Ctx.vWallNormal));
	_vector vTopN = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector vLeft{}, vRight{}, vN{};
	m_pEnvQueryCom->Compute_EdgeAnchor("TOP_LEFT_EDGE",  vEdge, vTrav, vTopN, vLeft,  vN);
	m_pEnvQueryCom->Compute_EdgeAnchor("TOP_RIGHT_EDGE", vEdge, vTrav, vTopN, vRight, vN);

	m_pIKDriverCom->Activate_Fixed("LeftArmTwoBone",  vLeft,  vN, EIKTARGET_MODE::POSITION, 1.f, 1.f, 0.4f);
	m_pIKDriverCom->Activate_Fixed("RightArmTwoBone", vRight, vN, EIKTARGET_MODE::POSITION, 1.f, 1.f, 0.4f);

	const HANG_TUNING& T = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;

	_vector vWallN = XMLoadFloat3(&Ctx.vWallNormal);
	m_pIKDriverCom->Activate_WallFoot("LeftLegTwoBone", vWallN, T.fFootPosWeight, T.fFootRotWeight, T.fFootBlendSec,
		T.fFootProbeOut, T.fFootProbeDepth, T.fFootSkin);
	m_pIKDriverCom->Activate_WallFoot("RightLegTwoBone", vWallN, T.fFootPosWeight, T.fFootRotWeight, T.fFootBlendSec,
		T.fFootProbeOut, T.fFootProbeDepth, T.fFootSkin);
}

void CTraceurClimbHang::OnExit()
{
	__super::OnExit();
	m_pMotionWarpCom->End_CurveWarp();

	m_pIKDriverCom->Deactivate("LeftArmTwoBone", 0.2f);
	m_pIKDriverCom->Deactivate("RightArmTwoBone", 0.2f);
	m_pIKDriverCom->Deactivate("LeftLegTwoBone", 0.2f);
	m_pIKDriverCom->Deactivate("RightLegTwoBone", 0.2f);
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

		Ctx.isValid      = true;
		Ctx.vGrabEdgePos = Scan.Reach.vEdgePos;
		Ctx.GrabBodyID   = Scan.Reach.HitBodyID;
		XMStoreFloat3(&Ctx.vWallNormal, XMVector3Normalize(vN));
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

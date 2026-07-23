#include "ClientPch.h"
#include "TraceurClimbEnter.h"
#include "AnimationController.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"
#include "GameSystem.h"
#include "ParkourTuningTable.h"
#include "ParkourMath.h"

HRESULT CTraceurClimbEnter::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurClimbEnter::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);

	if (!Ready_Enter(pArg))
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}
}

void CTraceurClimbEnter::OnExit()
{
	__super::OnExit();
	m_pMotionWarpCom->End_CurveWarp();
}

void CTraceurClimbEnter::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta * 1.5f);
}

void CTraceurClimbEnter::Late_Anim_Update(_float fTimeDelta)
{

	_float fCurveT = min(m_pAnimCtrlCom->Get_TrackPosition() / m_pModelCom->Get_Duration(
		m_pAnimCtrlCom->Get_CurrentAnimData()->AnimPlayDesc.strAnimationName), 1.f);
	m_pMotionWarpCom->Update_CurveWarp(fCurveT);
}

_bool CTraceurClimbEnter::Ready_Enter(void* pArg)
{
 	const PARKOUR_DECISION& Decision   = Enter_Decision(pArg);
	const ENV_PERCEPTION&   Perception = Enter_Perception(pArg);

	m_isHangEnter = (Decision.eCommand == PARKOUR_ACTION::HANG);
	if (m_isHangEnter)
	{
		if (!Ready_HangEnter(Perception))
			return false;
		return Select_Animation();
	}

	if (!Decision.isValid || !Perception.Scan.HeadHit.isHit)
		return false;

	if (!Select_Animation())
		return false;

	const OBSTACLE_SCAN& Scan = Perception.Scan;
	_float fColliderRadius = m_pColliderCom->Get_Radius();

	_vector vP0         = m_pTransformCom->Get_State(Engine::STATE::POSITION);
	_vector vWallNormal = XMVector3Normalize(XMLoadFloat3(&Scan.HeadHit.vHitNormal));
	_vector vP2         = XMVectorSetW(XMLoadFloat3(&Scan.HeadHit.vHitPosition), 1.f);
	vP2 = XMVectorSetY(vP2 + (vWallNormal * (fColliderRadius + 0.1f)), XMVectorGetY(vP0));

	m_pMotionWarpCom->Begin_CurveWarp(vP0, vP2, 0.3f,
		m_pTransformCom->Get_State(Engine::STATE::LOOK), -vWallNormal);

#ifdef _DEBUG
	XMStoreFloat3(&m_vDebugWallNormal, vWallNormal);
	m_vDebugWallHitPos = Scan.HeadHit.vHitPosition;
	XMStoreFloat3(&m_vDebugWallEndPos, XMLoadFloat3(&m_vDebugWallHitPos) + vWallNormal);
#endif
	return true;
}

_bool CTraceurClimbEnter::Ready_HangEnter(const ENV_PERCEPTION& Perception)
{
	const OBSTACLE_SCAN& Scan = Perception.Scan;
	if (!Scan.Reach.Hit.isHit || !Scan.Reach.hasEdge)
		return false;

	_vector vNormal = XMVectorSetY(XMLoadFloat3(&Scan.Reach.Hit.vHitNormal), 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vNormal)) < 1e-4f)
		return false;
	vNormal = XMVector3Normalize(vNormal);

	Set_HangContext(XMLoadFloat3(&Scan.Reach.vEdgePos), vNormal, Scan.Reach.HitBodyID);
	const HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();

	const HANG_TUNING&  T     = CGameSystem::GetInstance()->Get_ParkourTuning()->Get().Hang;
	const BODY_PROFILE* pBody = m_pOwner->Get_BodyProfile();

	_vector vP0 = m_pTransformCom->Get_State(Engine::STATE::POSITION);
	_vector vP2 = ParkourMath::Calc_HangPos(XMLoadFloat3(&Ctx.vGrabEdgePos), vNormal,
		pBody->fRadius, pBody->fHeight, T.fHangOffsetMult, T.fWallOffset);

	m_pMotionWarpCom->Begin_CurveWarp(vP0, vP2, 0.3f,
		m_pTransformCom->Get_State(Engine::STATE::LOOK), XMVectorNegate(vNormal));
	return true;
}

void CTraceurClimbEnter::Check_State()
{
	// Blackboard bool은 전환 미발생 프레임마다 전체 Clear되므로 매 프레임 재설정 (Ctx.Hang)
	Set_Flag("Ctx.Hang", m_isHangEnter);
}

_bool CTraceurClimbEnter::Select_Animation()
{
	Request_Anim(ENUM_CLASS(ETraceurClimbEnter::IdleToBracedHang));
	return true;
}

CTraceurClimbEnter* CTraceurClimbEnter::Create(CTraceur* pOwner)
{
	CTraceurClimbEnter* pInstance = new CTraceurClimbEnter();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbEnter");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbEnter::Free()
{
	__super::Free();
}

#include "ClientPch.h"
#include "TraceurClimbEnter.h"
#include "AnimationController.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"

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

	if (!Ready_Enter())
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

_bool CTraceurClimbEnter::Ready_Enter()
{
	if (!m_Decision.isValid || !m_Perception.Scan.HeadHit.isHit)
		return false;

	if (!Select_Animation())
		return false;

	const OBSTACLE_SCAN& Scan = m_Perception.Scan;
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

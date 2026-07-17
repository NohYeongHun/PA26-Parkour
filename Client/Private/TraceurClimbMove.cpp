#include "ClientPch.h"
#include "TraceurClimbMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbMove::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("KneeHit");
	Register_Flag("Mantle");

	return S_OK;
}

void CTraceurClimbMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	Request_Anim(ENUM_CLASS(ETraceurClimbMove::Move));
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB);
	Clear_Flags();
}

void CTraceurClimbMove::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurClimbMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const OBSTACLE_SCAN& Scan = m_pEnvQueryCom->Get_Perception().Scan;

	ACTORDIR eDir = Scan.ChestHit.isHit
		? CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		: ACTORDIR::END;

	if (Scan.ChestHit.isHit)
	{
		_vector vNormal   = XMLoadFloat3(&Scan.ChestHit.vHitNormal);
		_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
			eDir, vNormal, XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_pMoveCom->Move(vClimbDir, fTimeDelta, 1.f);
	}

	m_pMoveCom->Update_ClimbBlendWeight(eDir, fTimeDelta);
}

CTraceurClimbMove* CTraceurClimbMove::Create(CTraceur* pOwner)
{
	CTraceurClimbMove* pInstance = new CTraceurClimbMove();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbMove");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbMove::Free()
{
	__super::Free();
}

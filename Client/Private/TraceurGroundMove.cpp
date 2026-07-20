#include "ClientPch.h"
#include "TraceurGroundMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "Collider.h"
#include "Model.h"

HRESULT CTraceurGroundMove::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurGroundMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	Clear_Flags();
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::GROUND);
}

void CTraceurGroundMove::OnExit()
{
	__super::OnExit();
}

void CTraceurGroundMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Intent.Run"))
		fTargetWeight = 1.5f;
	else if (Get_Flag("Intent.Move"))
		fTargetWeight = 1.f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurGroundMove::Check_Physics(_float fTimeDelta)
{
}

CTraceurGroundMove* CTraceurGroundMove::Create(CTraceur* pOwner)
{
	CTraceurGroundMove* pInstance = new CTraceurGroundMove();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundMove");
		return nullptr;
	}
	return pInstance;
}

void CTraceurGroundMove::Free()
{
	__super::Free();
}

#include "ClientPch.h"
#include "TraceurAirJump.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurAirJump::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("ExitOpen");

	return S_OK;
}


void CTraceurAirJump::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurAirJump::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirJump::Update_Animations(_float fTimeDelta)
{
	__super::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Run"))
		fTargetWeight = 0.5f;
	else if (Get_Flag("MoveInput"))
		fTargetWeight = 0.2f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	// 기존 움직임 외에 추가적인 움직임
	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurAirJump::Check_Physics(_float fTimeDelta)
{
}

CTraceurAirJump* CTraceurAirJump::Create(CTraceur* pOwner)
{
	CTraceurAirJump* pInstance = new CTraceurAirJump();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurAirJump");
		return nullptr;
	}

	return pInstance;
}

void CTraceurAirJump::Free()
{
	__super::Free();
}

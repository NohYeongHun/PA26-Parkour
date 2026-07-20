#include "ClientPch.h"
#include "TraceurAirFall.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurAirFall::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurAirFall::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, 0.2f);
	//_vector vDown = XMVectorSet(0.f, -1.f, 0.f, 0.f);
	//m_pTransformCom->Go_Dir(vDown, fTimeDelta * 0.2f);
}

CTraceurAirFall* CTraceurAirFall::Create(CTraceur* pOwner)
{
	CTraceurAirFall* pInstance = new CTraceurAirFall();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurAirFall");
		return nullptr;
	}

	return pInstance;
}

void CTraceurAirFall::Free()
{
	__super::Free();
}

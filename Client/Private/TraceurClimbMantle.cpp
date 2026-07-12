#include "ClientPch.h"
#include "TraceurClimbMantle.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbMantle::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurClimbMantle::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurClimbMantle::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurClimbMantle::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbMantle::SetUp_Animations()
{
}

CTraceurClimbMantle* CTraceurClimbMantle::Create(CTraceur* pOwner)
{
	CTraceurClimbMantle* pInstance = new CTraceurClimbMantle();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbMantle");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbMantle::Free()
{
	__super::Free();
}

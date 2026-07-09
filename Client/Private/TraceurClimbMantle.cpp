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

	State_Reset();
}

void CTraceurClimbMantle::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	// 1. 입력 확인
	Check_State();

	// 2. 애니메이션 업데이트
	Update_Animations(fTimeDelta);

	// 3. 물리 체크
	Check_Physics(fTimeDelta);

	// 4. 상태 전환
	Check_StateTransition(fTimeDelta);
	
	// 5. 상태 초기화
	State_Reset();
}

void CTraceurClimbMantle::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurClimbMantle::Check_State()
{
}

void CTraceurClimbMantle::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbMantle::Check_Physics(_float fTimeDelta)
{
}

void CTraceurClimbMantle::Check_StateTransition(_float fTimeDelta)
{
}

void CTraceurClimbMantle::SetUp_Animations()
{
		
}

void CTraceurClimbMantle::State_Reset()
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




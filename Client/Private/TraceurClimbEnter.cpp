#include "ClientPch.h"
#include "TraceurClimbEnter.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbEnter::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurClimbEnter::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);

	State_Reset();
}

void CTraceurClimbEnter::OnUpdate(_float fTimeDelta)
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

void CTraceurClimbEnter::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurClimbEnter::Check_State()
{
}

void CTraceurClimbEnter::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbEnter::Check_Physics(_float fTimeDelta)
{
}

void CTraceurClimbEnter::Check_StateTransition(_float fTimeDelta)
{
}

void CTraceurClimbEnter::SetUp_Animations()
{
		
}

void CTraceurClimbEnter::State_Reset()
{
	
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




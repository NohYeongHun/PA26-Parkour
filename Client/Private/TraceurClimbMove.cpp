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

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurClimbMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbMove::HangingIdle);
	State_Reset();
}

void CTraceurClimbMove::OnUpdate(_float fTimeDelta)
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

void CTraceurClimbMove::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurClimbMove::Check_State()
{
}

void CTraceurClimbMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbMove::Check_Physics(_float fTimeDelta)
{
}

void CTraceurClimbMove::Check_StateTransition(_float fTimeDelta)
{
	// 임시.
	if (m_IsAnimationEnd)
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::CLIMB),
			ENUM_CLASS(ETraceurClimbState::Move));
	}
}

void CTraceurClimbMove::SetUp_Animations()
{
	CState::Add_ParkourAnimations(ENUM_CLASS(ETraceurClimbMove::HangingIdle),
		{ &m_fTrackPosition, "HangingIdle", 1.f, 0.2f, 0.f, false }, { 1.f, false, true, true }, {});
}

void CTraceurClimbMove::State_Reset()
{
	
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




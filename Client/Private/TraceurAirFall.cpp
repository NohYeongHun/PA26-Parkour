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

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurAirFall::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	// 어디서 낙하가 시작됐는지에 따라 재생할 애니메이션을 다르게 선택
	const auto& prevKey = m_pStateMachinCom->Get_PrevStateKey();
	if (EStateCategory::CLIMB == static_cast<EStateCategory>(prevKey.iCategory) &&
		ETraceurClimbState::Move == static_cast<ETraceurClimbState>(prevKey.iSubState))
	{
		m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirFall::JumpFromWall);
	}
	else
	{
		m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirFall::FallingIdle);
	}

	m_pColliderCom->Set_Gravity(true);
	State_Reset();
}

void CTraceurAirFall::OnUpdate(_float fTimeDelta)
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

void CTraceurAirFall::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::Check_State()
{
	m_States[LAND] = m_pColliderCom->IsLand();
}

void CTraceurAirFall::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurAirFall::Check_Physics(_float fTimeDelta)
{
}

void CTraceurAirFall::Check_StateTransition(_float fTimeDelta)
{
	ETraceurAirFall eType = static_cast<ETraceurAirFall>(m_iCurrentAnimIdx);

	if (eType == ETraceurAirFall::JumpFromWall)
	{
		if (m_IsAnimationEnd)
		{
			if (m_States[LAND])
			{
				m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND),
					ENUM_CLASS(ETraceurGroundState::Land));
				return;
			}
		}
	}

	if (eType == ETraceurAirFall::FallingIdle)
	{
		if (m_States[LAND])
		{
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND),
				ENUM_CLASS(ETraceurGroundState::Land));
			return;
		}
	}
}

void CTraceurAirFall::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::FallingIdle),
		{ &m_fTrackPosition, "FallingIdle", 1.f, 0.05f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::JumpFromWall),
		{ &m_fTrackPosition, "JumpFromWall", 1.f, 0.05f, 0.f, false }, { 1.f, true, true, true });
}

void CTraceurAirFall::State_Reset()
{
	
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




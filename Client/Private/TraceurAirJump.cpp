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

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurAirJump::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirFall::FallingIdle);
	m_pColliderCom->Set_Gravity(true);
	State_Reset();


}

void CTraceurAirJump::OnUpdate(_float fTimeDelta)
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

void CTraceurAirJump::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurAirJump::Check_State()
{
	m_States[LAND] = m_pColliderCom->IsLand();
}

void CTraceurAirJump::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
	_float fTargetWeight = 0.f;
	if (m_States[RUN])
		fTargetWeight = 0.5f;
	else if (m_States[MOVE])
		fTargetWeight = 0.2f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurAirJump::Check_Physics(_float fTimeDelta)
{
}

void CTraceurAirJump::Check_StateTransition(_float fTimeDelta)
{
	/*if (m_States[LAND])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND),
			ENUM_CLASS(ETraceurGroundState::Land));
		return;
	}*/

	if (m_IsAnimationEnd)
	{
		if (!m_States[LAND])
		{
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::AIR),
				ENUM_CLASS(ETraceurAirState::Fall));
			return;
		}

		if (m_States[LAND])
		{
			m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND),
				ENUM_CLASS(ETraceurGroundState::Land));
			return;
		}

		
	}
}

void CTraceurAirJump::SetUp_Animations()
{
	
	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::FallingIdle),
		{ &m_fTrackPosition, "Jump", 1.f, 0.1f, 0.f, false }, { 1.f, true, true, true });
}

void CTraceurAirJump::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
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




#include "ClientPch.h"
#include "TraceurGroundLand.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurGroundLand::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundLand::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	State_Reset();

	const auto& prevKey = m_pStateMachinCom->Get_PrevStateKey();
	if (EStateCategory::AIR == static_cast<EStateCategory>(prevKey.iCategory) &&
		ETraceurAirState::Fall == static_cast<ETraceurAirState>(prevKey.iSubState))
	{
		if (m_pStateMachinCom->Get_PrevAnimIndex() == ENUM_CLASS(ETraceurAirFall::FallingIdle))
			m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundLand::FallingToLanding);
		else if (m_pStateMachinCom->Get_PrevAnimIndex() == ENUM_CLASS(ETraceurAirFall::JumpFromWall))
			m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundLand::FallingToLanding);
	}
	else
	{
		m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirFall::FallingIdle);
	}
	
}

void CTraceurGroundLand::OnUpdate(_float fTimeDelta)
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

void CTraceurGroundLand::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundLand::Check_State()
{
}

void CTraceurGroundLand::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundLand::Check_Physics(_float fTimeDelta)
{
}

void CTraceurGroundLand::Check_StateTransition(_float fTimeDelta)
{
	if (m_IsAnimationEnd)
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}
		
	
}

void CTraceurGroundLand::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundLand::FallingToLanding),
		{ &m_fTrackPosition, "FallingToLanding", 1.f, 0.05f, 0.f, false }, { 1.f, true, true, true });
}

void CTraceurGroundLand::State_Reset()
{
	
}



CTraceurGroundLand* CTraceurGroundLand::Create(CTraceur* pOwner)
{
	CTraceurGroundLand* pInstance = new CTraceurGroundLand();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundLand");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundLand::Free()
{
	__super::Free();
}




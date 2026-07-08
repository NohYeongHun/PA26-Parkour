#include "ClientPch.h"
#include "TraceurGroundWalk.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"

HRESULT CTraceurGroundWalk::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundWalk::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	// 1. 전환되었을 때 조건을 확인하고 현재 애니메이션을 선택한다.
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundWalk::Walking);
	

	State_Reset();
}

void CTraceurGroundWalk::OnUpdate(_float fTimeDelta)
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

void CTraceurGroundWalk::OnExit()
{
	__super::OnExit();
}

void CTraceurGroundWalk::Check_State()
{
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN] = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}

void CTraceurGroundWalk::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_vector vWorldDir = CMovementComponent::Calc_WorldDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());
	m_pMoveCom->Move(vWorldDir, fTimeDelta, 0.3f);
}

void CTraceurGroundWalk::Check_Physics(_float fTimeDelta)
{
}

void CTraceurGroundWalk::Check_StateTransition(_float fTimeDelta)
{
	// 1. Run 입력시 변경
	if (m_States[RUN])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Run));
		return;
	}

	// 2. Move 입력 중이면 return; => 다시 실행하도록?
	if (m_States[MOVE])
	{
		return;
	}

	// 아무것도 안하면 자동으로 Idle
	m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Idle));
		
}

void CTraceurGroundWalk::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundWalk::Walking),
		{&m_fTrackPosition, "Walking", 1.f, 0.05f, 0.f, false}, { 1.f, true, true, true});
}

void CTraceurGroundWalk::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}



CTraceurGroundWalk* CTraceurGroundWalk::Create(CTraceur* pOwner)
{
	CTraceurGroundWalk* pInstance = new CTraceurGroundWalk();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundWalk");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundWalk::Free()
{
	__super::Free();
}




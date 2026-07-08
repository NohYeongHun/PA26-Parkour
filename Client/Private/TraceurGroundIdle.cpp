#include "ClientPch.h"
#include "TraceurGroundIdle.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"

HRESULT CTraceurGroundIdle::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundIdle::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundIdle::Idle);

	State_Reset();
}

void CTraceurGroundIdle::OnUpdate(_float fTimeDelta)
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

#ifdef _DEBUG
	if (m_States[MOVE])
		cout << "이동 키 누름 " << endl;
#endif // _DEBUG
	
	// 5. 상태 초기화
	State_Reset();



}

void CTraceurGroundIdle::OnExit()
{
	__super::OnExit();
}

void CTraceurGroundIdle::Check_State()
{
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN] = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}

void CTraceurGroundIdle::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundIdle::Check_Physics(_float fTimeDelta)
{
}

void CTraceurGroundIdle::Check_StateTransition(_float fTimeDelta)
{
	// 1. 입력 시 변경
	if (m_States[RUN])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Run));
		return;
	}

	// 2. Move 우선순위
	if (m_States[MOVE])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Walk));
		return;
	}


	// last 입력이나 상태 변경 없이 종료 되면
	if (m_IsAnimationEnd)
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Idle));
}

void CTraceurGroundIdle::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundIdle::Idle),
		{&m_fTrackPosition, "Idle", 1.f, 0.2f, 0.f, false}, { 1.f, true, true, true});
}

void CTraceurGroundIdle::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}



CTraceurGroundIdle* CTraceurGroundIdle::Create(CTraceur* pOwner)
{
	CTraceurGroundIdle* pInstance = new CTraceurGroundIdle();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundIdle");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundIdle::Free()
{
	__super::Free();
}




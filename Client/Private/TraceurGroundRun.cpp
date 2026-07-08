#include "ClientPch.h"
#include "TraceurGroundRun.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurGroundRun::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundRun::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	// 1. 전환되었을 때 조건을 확인하고 현재 애니메이션을 선택한다.
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundRun::Run);
	

	State_Reset();
}

void CTraceurGroundRun::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	// 1. 상태 확인.
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

void CTraceurGroundRun::OnExit()
{
	__super::OnExit();
}

void CTraceurGroundRun::Check_State()
{
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN] = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
	
}

void CTraceurGroundRun::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_vector vWorldDir = CMovementComponent::Calc_WorldDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());
	m_pMoveCom->Move(vWorldDir, fTimeDelta, 0.5f);
}

void CTraceurGroundRun::Check_Physics(_float fTimeDelta)
{
	const ENV_QUERY_RESULT& EnvResult = m_pEnvQueryCom->Get_QueryResult();
	m_States[VAULT] = m_States[MOVE] && EnvResult.isValid
		&& EnvResult.eBestAction == PARKOUR_ACTION::VAULT;
}

void CTraceurGroundRun::Check_StateTransition(_float fTimeDelta)
{
	if (m_States[VAULT])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Vault));
		return;
	}

	// Run 입력 중이면 실행 중 작업을 계속 실행
	if (m_States[RUN])
	{
		return;
	}

	m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Idle));
		
}

void CTraceurGroundRun::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundRun::Run),
		{&m_fTrackPosition, "Run", 1.f, 0.2f, 0.f, false}, { 1.f, true, true, true});
}

void CTraceurGroundRun::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}



CTraceurGroundRun* CTraceurGroundRun::Create(CTraceur* pOwner)
{
	CTraceurGroundRun* pInstance = new CTraceurGroundRun();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundRun");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundRun::Free()
{
	__super::Free();
}




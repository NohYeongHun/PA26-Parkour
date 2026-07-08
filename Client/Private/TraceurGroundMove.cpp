#include "ClientPch.h"
#include "TraceurGroundMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurGroundMove::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundMove::Move);

	return S_OK;
}

void CTraceurGroundMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	State_Reset();
}

void CTraceurGroundMove::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	Check_State();
	Update_Animations(fTimeDelta);
	Check_Physics(fTimeDelta);
	Check_StateTransition(fTimeDelta);
	State_Reset();
}

void CTraceurGroundMove::OnExit()
{
	__super::OnExit();
}

void CTraceurGroundMove::Check_State()
{
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN]  = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}

void CTraceurGroundMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	// 목표 가중치: 무입력 0 / 이동 0.5 / 이동+LSHIFT 1.0
	_float fTargetWeight = 0.f;
	if (m_States[RUN])
		fTargetWeight = 1.f;
	else if (m_States[MOVE])
		fTargetWeight = 0.5f;

	_vector vWorldDir = CMovementComponent::Calc_WorldDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurGroundMove::Check_Physics(_float fTimeDelta)
{
	const ENV_QUERY_RESULT& EnvResult = m_pEnvQueryCom->Get_QueryResult();
	m_States[VAULT] = m_States[MOVE] && m_States[RUN]
		&& EnvResult.isValid && EnvResult.eBestAction == PARKOUR_ACTION::VAULT;
}

void CTraceurGroundMove::Check_StateTransition(_float fTimeDelta)
{
	if (m_States[VAULT])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Vault));
	}
}

void CTraceurGroundMove::SetUp_Animations()
{
	BLENDSPACE_1D_DESC bs{};
	bs.pParam   = m_pMoveCom->Get_LocomotionWeightPtr();
	bs.fBlendDuration = 0.2f;
	bs.Samples  = { {"Idle", 0.f}, {"Walk", 0.5f}, {"Run", 1.f} };

	ROOTMOTION_DESC root{};

	Add_BlendSpace(ENUM_CLASS(ETraceurGroundMove::Move), bs, root);
}

void CTraceurGroundMove::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}

CTraceurGroundMove* CTraceurGroundMove::Create(CTraceur* pOwner)
{
	CTraceurGroundMove* pInstance = new CTraceurGroundMove();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundMove");
		return nullptr;
	}
	return pInstance;
}

void CTraceurGroundMove::Free()
{
	__super::Free();
}

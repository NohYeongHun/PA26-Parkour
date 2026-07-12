#include "ClientPch.h"
#include "TraceurGroundMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "Collider.h"
#include "Model.h"

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
	m_pColliderCom->Set_Gravity(true);
	State_Reset();
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::GROUND);
	m_fFallTime = 0.f;
}

void CTraceurGroundMove::OnExit()
{
	__super::OnExit();
	m_fFallTime = 0.f;
}

void CTraceurGroundMove::Check_State()
{
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN]  = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
	m_States[LAND] = m_pColliderCom->IsLand();
	m_States[JUMP] = m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
}

void CTraceurGroundMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (m_States[RUN])
		fTargetWeight = 1.f;
	else if (m_States[MOVE])
		fTargetWeight = 0.5f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurGroundMove::Check_Physics(_float fTimeDelta)
{
	m_States[LAND] = m_pColliderCom->IsLand();
	if (m_States[LAND])
	{
		m_fFallTime = 0.f;
	}
	else
	{
		m_fFallTime += fTimeDelta;
		if (m_fFallTime >= 0.2f)
			m_States[FALL] = true;
	}

	const ENV_QUERY_RESULT& EnvResult = m_pEnvQueryCom->Get_QueryResult();
	if (!m_States[MOVE] || !m_States[RUN])
		return;

	if (EnvResult.isValid)
	{
		if (EnvResult.eBestAction == PARKOUR_ACTION::LOW_VAULT)
		{
			m_States[VAULT] = true;
			return;
		}

		if (EnvResult.eBestAction == PARKOUR_ACTION::CLIMB)
		{
			m_States[CLIMB] = true;
			return;
		}
	}
}

void CTraceurGroundMove::SetUp_Animations()
{
	BLENDSPACE_1D_DESC bs{};
	bs.pParam         = m_pMoveCom->Get_LocomotionWeightPtr();
	bs.fBlendDuration = 0.2f;
	bs.Samples        = { {"Idle", 0.f}, {"Walk", 0.5f}, {"Run", 1.f} };

	ROOTMOTION_DESC root{};
	root.fRate = 1.f;

	Add_BlendSpace(ENUM_CLASS(ETraceurGroundMove::Move), bs, root);
}

void CTraceurGroundMove::SetUp_Transitions()
{
	Add_Transition(
		[this] { return m_States[FALL]; },
		{ ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall) },
		ENUM_CLASS(ETraceurAirFall::FallALoop)
	);
	Add_Transition(
		[this] { return m_States[JUMP]; },
		{ ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Jump) }
	);
	Add_Transition(
		[this] { return m_States[CLIMB]; },
		{ ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Enter) }
	);
	Add_Transition(
		[this] { return m_States[VAULT]; },
		{ ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Vault) }
	);
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

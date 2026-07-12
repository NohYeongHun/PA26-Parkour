#include "ClientPch.h"
#include "TraceurState.h"
#include "Traceur.h"
#include "Model.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "TraceurState_Enum.h"


HRESULT CTraceurState::Initialize(CTraceur* pOwner)
{
	m_pOwner = pOwner;
	if (nullptr == pOwner)
		return E_FAIL;

	m_pModelCom = dynamic_cast<CModel*>(pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	m_pColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pColliderCom)
		return E_FAIL;

	m_pStateMachinCom = dynamic_cast<CStateMachine*>(m_pOwner->Get_Component(TEXT("Com_StateMachine")));
	if (nullptr == m_pStateMachinCom)
		return E_FAIL;

	m_pInputControllerCom = dynamic_cast<CInputController*>(m_pOwner->Get_Component(TEXT("Com_InputController")));
	if (nullptr == m_pInputControllerCom)
		return E_FAIL;

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	if (nullptr == m_pEnvQueryCom)
		return E_FAIL;

	m_pMoveCom = dynamic_cast<CMovementComponent*>(m_pOwner->Get_Component(TEXT("Com_Move")));
	if (nullptr == m_pMoveCom)
		return E_FAIL;

	m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);

	SetUp_Transitions();

	return S_OK;
}

void CTraceurState::OnEnter(void* pArg)
{
	CState::OnEnter(pArg);
	if (pArg)
	{
		auto* pDesc = static_cast<STATE_ENTER_DESC*>(pArg);
		if (pDesc->iAnimIndex != UINT_MAX)
			m_iCurrentAnimIdx = pDesc->iAnimIndex;
	}
}

void CTraceurState::OnUpdate(_float fTimeDelta)
{
	CState::OnUpdate(fTimeDelta);
	Check_State();
	Update_Animations(fTimeDelta);
	Late_Anim_Update(fTimeDelta);
	Check_Physics(fTimeDelta);
	Evaluate_Transitions();
	State_Reset();
}

void CTraceurState::OnExit()
{
	CState::OnExit();
}

_bool CTraceurState::IsVault() const
{
	const auto& stateKey = m_pStateMachinCom->Get_CurrentStateKey();
	return (EStateCategory::GROUND == static_cast<EStateCategory>(stateKey.iCategory) &&
		ETraceurGroundState::Vault == static_cast<ETraceurGroundState>(stateKey.iSubState));
}

_bool CTraceurState::Play_Animation(_float fTimeDelta)
{
	const auto& iter = m_Animations.find(m_iCurrentAnimIdx);
	if (iter == m_Animations.end())
		return false;

	if (iter->second.eType == EAnimSlotType::BLENDSPACE_1D)
	{
		m_pModelCom->Play_BlendSpace_CPU(iter->second.BlendSpaceDesc, iter->second.RootMotionDesc, fTimeDelta);
		m_IsAnimationEnd = false;
	}
	else if (iter->second.eType == EAnimSlotType::BLENDSPACE_2D)
	{
		m_pModelCom->Play_BlendSpace2D_CPU(iter->second.BlendSpace2Desc, iter->second.RootMotionDesc, fTimeDelta);
		m_IsAnimationEnd = false;
	}
	else
	{
		m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(iter->second.AnimPlayDesc, iter->second.RootMotionDesc, fTimeDelta);
	}

	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(Engine::STATE::POSITION));

	return true;
}

void CTraceurState::Add_Transition(function<_bool()> Condition, Engine::StateKey Next, _uint iNextAnim)
{
	m_Transitions.push_back({ move(Condition), Next, iNextAnim, nullptr });
}

void CTraceurState::Add_Transition(function<_bool()> Condition, Engine::StateKey Next, shared_ptr<STATE_ENTER_DESC> pDesc)
{
	m_Transitions.push_back({ move(Condition), Next, UINT_MAX, move(pDesc) });
}

void CTraceurState::Evaluate_Transitions()
{
	for (auto& rule : m_Transitions)
	{
		if (!rule.Condition()) continue;

		if (rule.pDesc)
			m_pStateMachinCom->Change_State(rule.Next.iCategory, rule.Next.iSubState, rule.pDesc.get());
		else if (rule.iNextAnim != UINT_MAX)
		{
			STATE_ENTER_DESC desc{ rule.iNextAnim };
			m_pStateMachinCom->Change_State(rule.Next.iCategory, rule.Next.iSubState, &desc);
		}
		else
		{
			m_pStateMachinCom->Change_State(rule.Next.iCategory, rule.Next.iSubState);
		}
		return;
	}
}

void CTraceurState::Free()
{
	__super::Free();
}

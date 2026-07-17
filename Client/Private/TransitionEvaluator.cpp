#include "ClientPch.h"
#include "TransitionEvaluator.h"
#include "StateBlackboard.h"
#include "TransitionTable.h"
#include "GameSystem.h"
#include "Model.h"
#include "AnimationController.h"
#include "EnvironmentQueryComponent.h"
#include "ParkourDeciderComponent.h"
#include "TraceurStateNames.h"

CTransitionEvaluator::CTransitionEvaluator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CTransitionEvaluator::CTransitionEvaluator(const CTransitionEvaluator& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CTransitionEvaluator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTransitionEvaluator::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pBlackboardCom = dynamic_cast<CStateBlackboard*>(m_pOwner->Get_Component(TEXT("Com_StateBlackboard")));
	if (nullptr == m_pBlackboardCom) return E_FAIL;

	m_pStateMachineCom = dynamic_cast<Engine::CStateMachine*>(m_pOwner->Get_Component(TEXT("Com_StateMachine")));
	if (nullptr == m_pStateMachineCom) return E_FAIL;

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	if (nullptr == m_pEnvQueryCom) return E_FAIL;

	m_pDeciderCom = dynamic_cast<CParkourDeciderComponent*>(m_pOwner->Get_Component(TEXT("Com_ParkourDecider")));
	if (nullptr == m_pDeciderCom) return E_FAIL;

	m_pModelCom = dynamic_cast<CModel*>(m_pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pModelCom) return E_FAIL;

	m_pAnimCtrlCom = dynamic_cast<Engine::CAnimationController*>(m_pOwner->Get_Component(TEXT("Com_AnimController")));
	if (nullptr == m_pAnimCtrlCom) return E_FAIL;

	return S_OK;
}

void CTransitionEvaluator::Bind_Rules(const CTransitionTable* pTable)
{
	m_BoundRules.clear();
	m_iBoundVersion = pTable->Get_Version();

	_string strWarnings;

	for (const auto& StatePair : pTable->Get_All())
	{
		const Engine::StateKey& Key = StatePair.first;
		vector<BOUND_TRANSITION> Bounds;

		for (const TRANSITION_RULE_DATA& Data : StatePair.second)
		{
			BOUND_TRANSITION Bound{};
			Bound.iAnimGuard     = Data.iAnimGuard;
			Bound.Next           = Data.Next;
			Bound.iNextAnim      = Data.iNextAnim;
			Bound.fBlendOverride = Data.fBlendOverride;

			auto ResolveSlots = [this, &Bound, &Key, &strWarnings](const vector<_string>& Names, vector<_uint>& OutSlots)
			{
				for (const _string& strName : Names)
				{
					const _uint iSlot = m_pBlackboardCom->Find_Slot(strName);
					if (iSlot == UINT_MAX)
					{
						strWarnings += CTraceurStateNames::To_String(Key)
							+ ": 등록되지 않은 플래그 \"" + strName + "\" — 해당 규칙 비활성화\n";
						Bound.isDisabled = true;
						return;
					}
					OutSlots.push_back(iSlot);
				}
			};

			ResolveSlots(Data.WhenFlags, Bound.WhenSlots);
			if (!Bound.isDisabled)
				ResolveSlots(Data.WhenNotFlags, Bound.WhenNotSlots);

			if (!Bound.isDisabled)
				Bounds.push_back(move(Bound));
		}

		if (!Bounds.empty())
			m_BoundRules.emplace(Key, move(Bounds));
	}

	if (!strWarnings.empty())
		MessageBoxA(nullptr, ("전환 규칙 바인딩 경고:\n" + strWarnings).c_str(), "TransitionTable Bind", MB_OK);
}

_bool CTransitionEvaluator::Evaluate()
{
	CTransitionTable* pTable = CGameSystem::GetInstance()->Get_TransitionTable();
	if (nullptr == pTable)
		return false;

	if (m_iBoundVersion != pTable->Get_Version())
		Bind_Rules(pTable);

	const Engine::StateKey CurKey = m_pStateMachineCom->Get_CurrentStateKey();
	const auto itRules = m_BoundRules.find(CurKey);
	if (itRules == m_BoundRules.end())
		return false;

	const _uint iCurrentAnim = m_pAnimCtrlCom->Get_CurrentAnimId();

	for (const BOUND_TRANSITION& Rule : itRules->second)
	{
		if (Rule.isDisabled)
			continue;
		if (Rule.iAnimGuard != UINT_MAX && iCurrentAnim != Rule.iAnimGuard)
			continue;

		_bool isMatch = true;
		for (_uint iSlot : Rule.WhenSlots)
			if (!m_pBlackboardCom->Get(iSlot)) { isMatch = false; break; }
		if (isMatch)
			for (_uint iSlot : Rule.WhenNotSlots)
				if (m_pBlackboardCom->Get(iSlot)) { isMatch = false; break; }
		if (!isMatch)
			continue;

		if (Rule.fBlendOverride >= 0.f)
			m_pModelCom->Set_NextBlendOverride(Rule.fBlendOverride);

#ifdef _DEBUG
		{
			const _string strFrom = CTraceurStateNames::To_String(CurKey);
			const _string strTo   = CTraceurStateNames::To_String(Rule.Next);
			cout << "[Transition] " << strFrom << " -> " << strTo << "\n";
		}
#endif

		STATE_ENTER_DESC Desc{};
		Desc.iAnimIndex     = Rule.iNextAnim;
		Desc.hasEnvSnapshot = true;
		Desc.Perception     = m_pEnvQueryCom->Get_Perception();
		Desc.Decision       = m_pDeciderCom->Get_Decision();
		m_pStateMachineCom->Change_State(Rule.Next.iCategory, Rule.Next.iSubState, &Desc);
		return true;
	}
	return false;
}

CTransitionEvaluator* CTransitionEvaluator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTransitionEvaluator* pInstance = new CTransitionEvaluator(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTransitionEvaluator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CTransitionEvaluator::Clone(void* pArg)
{
	CTransitionEvaluator* pInstance = new CTransitionEvaluator(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CTransitionEvaluator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTransitionEvaluator::Free()
{
	__super::Free();
}

#include "ClientPch.h"
#include "TransitionEvaluator.h"
#include "StateBlackboard.h"
#include "TransitionTable.h"
#include "GameSystem.h"
#include "Animator.h"
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

	m_pAnimCtrlCom = dynamic_cast<Engine::CAnimationController*>(m_pOwner->Get_Component(TEXT("Com_AnimController")));
	if (nullptr == m_pAnimCtrlCom) return E_FAIL;

	m_pAnimator = m_pAnimCtrlCom->Get_Animator();
	if (nullptr == m_pAnimator) return E_FAIL;

	return S_OK;
}

void CTransitionEvaluator::Bind_One(const _string& strKeyName, const TRANSITION_RULE_DATA& Data,
	vector<BOUND_TRANSITION>& OutBounds, _string& strWarnings)
{
	BOUND_TRANSITION Bound{};
	Bound.iAnimGuard     = Data.iAnimGuard;
	Bound.Next           = Data.Next;
	Bound.iNextAnim      = Data.iNextAnim;
	Bound.fBlendOverride = Data.fBlendOverride;

	auto ResolveGroups = [this, &Bound, &strKeyName, &strWarnings](const vector<_string>& Names, vector<vector<_uint>>& OutGroups)
	{
		for (const _string& strName : Names)
		{
			vector<_uint> Group = m_pBlackboardCom->Collect_Slots(strName);
			if (Group.empty())
			{
				strWarnings += strKeyName + ": 등록되지 않은 태그 \"" + strName + "\" — 해당 규칙 비활성화\n";
				Bound.isDisabled = true;
				return;
			}
			OutGroups.push_back(move(Group));
		}
	};

	ResolveGroups(Data.WhenFlags, Bound.WhenGroups);
	if (!Bound.isDisabled)
		ResolveGroups(Data.WhenNotFlags, Bound.WhenNotGroups);

	if (!Bound.isDisabled)
		OutBounds.push_back(move(Bound));
}

void CTransitionEvaluator::Bind_Rules(const CTransitionTable* pTable)
{
	m_BoundRules.clear();
	m_BoundCategoryRules.clear();
	m_iBoundVersion = pTable->Get_Version();

	_string strWarnings;

	for (const auto& StatePair : pTable->Get_All())
	{
		vector<BOUND_TRANSITION> Bounds;
		for (const TRANSITION_RULE_DATA& Data : StatePair.second)
			Bind_One(CTraceurStateNames::To_String(StatePair.first), Data, Bounds, strWarnings);
		if (!Bounds.empty())
			m_BoundRules.emplace(StatePair.first, move(Bounds));
	}

	for (const auto& CategoryPair : pTable->Get_AllCategories())
	{
		// 경고 표기용 카테고리 이름: To_String은 "CLIMB/Enter" 형식이므로 '/' 앞부분만 사용
		const _string strFull = CTraceurStateNames::To_String(Engine::StateKey(CategoryPair.first, 0));
		const _string strCategoryName = strFull.substr(0, strFull.find('/'));

		vector<BOUND_TRANSITION> Bounds;
		for (const TRANSITION_RULE_DATA& Data : CategoryPair.second)
			Bind_One(strCategoryName, Data, Bounds, strWarnings);
		if (!Bounds.empty())
			m_BoundCategoryRules.emplace(CategoryPair.first, move(Bounds));
	}

	if (!strWarnings.empty())
		MessageBoxA(nullptr, ("전환 규칙 바인딩 경고:\n" + strWarnings).c_str(), "TransitionTable Bind", MB_OK);
}

_bool CTransitionEvaluator::Try_Fire(const BOUND_TRANSITION& Rule, _uint iCurrentAnim, const Engine::StateKey& CurKey)
{
	if (Rule.isDisabled)
		return false;
	if (Rule.iAnimGuard != UINT_MAX && iCurrentAnim != Rule.iAnimGuard)
		return false;

	_bool isMatch = true;
	for (const auto& Group : Rule.WhenGroups)
	{
		_bool isAnyOn = false;
		for (_uint iSlot : Group)
			if (m_pBlackboardCom->Get(iSlot)) { isAnyOn = true; break; }
		if (!isAnyOn) { isMatch = false; break; }
	}
	if (isMatch)
	{
		for (const auto& Group : Rule.WhenNotGroups)
		{
			for (_uint iSlot : Group)
				if (m_pBlackboardCom->Get(iSlot)) { isMatch = false; break; }
			if (!isMatch) break;
		}
	}
	if (!isMatch)
		return false;

	if (Rule.fBlendOverride >= 0.f)
		m_pAnimator->Set_NextBlendOverride(Rule.fBlendOverride);

#ifdef _DEBUG
	{
		const _string strFrom = CTraceurStateNames::To_String(CurKey);
		const _string strTo   = CTraceurStateNames::To_String(Rule.Next);
		cout << "[Transition] " << strFrom << " -> " << strTo
		     << "  ObjectFlag=" << ENUM_CLASS(m_pEnvQueryCom->Get_Perception().Scan.eObjectFlag)
		     << "  ReachFlag=" << ENUM_CLASS(m_pEnvQueryCom->Get_Perception().Scan.Reach.eObjectFlag) << "\n";
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

_bool CTransitionEvaluator::Evaluate()
{
	CTransitionTable* pTable = CGameSystem::GetInstance()->Get_TransitionTable();
	if (nullptr == pTable)
		return false;

	if (m_iBoundVersion != pTable->Get_Version())
		Bind_Rules(pTable);

	const Engine::StateKey CurKey = m_pStateMachineCom->Get_CurrentStateKey();
	const _uint iCurrentAnim = m_pAnimCtrlCom->Get_CurrentAnimId();

	// 1. 상태 규칙 (구체적 — 우선)
	const auto itRules = m_BoundRules.find(CurKey);
	if (itRules != m_BoundRules.end())
		for (const BOUND_TRANSITION& Rule : itRules->second)
			if (Try_Fire(Rule, iCurrentAnim, CurKey))
				return true;

	// 2. 카테고리 공통 규칙 (폴백)
	const auto itCategory = m_BoundCategoryRules.find(CurKey.iCategory);
	if (itCategory != m_BoundCategoryRules.end())
		for (const BOUND_TRANSITION& Rule : itCategory->second)
			if (Try_Fire(Rule, iCurrentAnim, CurKey))
				return true;

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

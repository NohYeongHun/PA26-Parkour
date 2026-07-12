#include "ClientPch.h"
#include "TraceurState.h"
#include "Traceur.h"
#include "Model.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "TraceurState_Enum.h"
#include "GameSystem.h"
#include "TransitionTable.h"
#include "TraceurStateNames.h"


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

	Register_Flag("AnimEnd"); // 내장 플래그 — 매 프레임 m_IsAnimationEnd 반영, 모든 상태 공용

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
	Set_Flag("AnimEnd", m_IsAnimationEnd);
	Evaluate_Transitions();
	// 플래그는 프레임 스코프: Check_* 세팅 → Evaluate 소비 → 즉시 초기화 (외부에서 Get_Flag 금지)
	Clear_Flags();
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

void CTraceurState::Register_Flag(const _string& strName)
{
	if (m_FlagSlots.find(strName) != m_FlagSlots.end())
		return;
	m_FlagSlots.emplace(strName, static_cast<_uint>(m_FlagValues.size()));
	m_FlagValues.push_back(false);
}

// 플래그는 Initialize의 Register_Flag로 선언된 것만 사용 가능 (오타 = 즉시 크래시).
// 데이터(JSON) 쪽 미등록 플래그는 Bind_Rules가 비활성화로 처리하므로 크래시하지 않는다.
void CTraceurState::Set_Flag(const _string& strName, _bool isOn)
{
	const auto it = m_FlagSlots.find(strName);
	ASSERT_CRASH(it != m_FlagSlots.end());
	m_FlagValues[it->second] = isOn;
}

_bool CTraceurState::Get_Flag(const _string& strName) const
{
	const auto it = m_FlagSlots.find(strName);
	ASSERT_CRASH(it != m_FlagSlots.end());
	return m_FlagValues[it->second];
}

void CTraceurState::Clear_Flags()
{
	fill(m_FlagValues.begin(), m_FlagValues.end(), false);
}

void CTraceurState::Bind_Rules(const CTransitionTable* pTable)
{
	m_BoundRules.clear();
	m_iBoundVersion = pTable->Get_Version();

	const vector<TRANSITION_RULE_DATA>* pRules = pTable->Get_Rules(m_SelfKey);
	if (nullptr == pRules)
		return;

	_string strWarnings;

	for (const TRANSITION_RULE_DATA& Data : *pRules)
	{
		BOUND_TRANSITION Bound{};
		Bound.iAnimGuard = Data.iAnimGuard;
		Bound.Next       = Data.Next;
		Bound.iNextAnim  = Data.iNextAnim;

		auto ResolveSlots = [this, &Bound, &strWarnings](const vector<_string>& Names, vector<_uint>& OutSlots)
		{
			for (const _string& strName : Names)
			{
				const auto it = m_FlagSlots.find(strName);
				if (it == m_FlagSlots.end())
				{
					strWarnings += "등록되지 않은 플래그 \"" + strName + "\" — 해당 규칙 비활성화\n";
					Bound.isDisabled = true;
					return;
				}
				OutSlots.push_back(it->second);
			}
		};

		ResolveSlots(Data.WhenFlags, Bound.WhenSlots);
		if (!Bound.isDisabled)
			ResolveSlots(Data.WhenNotFlags, Bound.WhenNotSlots);

		if (!Bound.isDisabled)
			m_BoundRules.push_back(move(Bound));
	}

	if (!strWarnings.empty())
	{
		const _string strMsg = CTraceurStateNames::To_String(m_SelfKey) + " 상태 규칙 바인딩 경고:\n" + strWarnings;
		MessageBoxA(nullptr, strMsg.c_str(), "TransitionTable Bind", MB_OK);
	}
}

void CTraceurState::Evaluate_Transitions()
{
	CTransitionTable* pTable = CGameSystem::GetInstance()->Get_TransitionTable();
	if (nullptr == pTable)
		return;

	if (m_iBoundVersion != pTable->Get_Version())
		Bind_Rules(pTable);

	for (const BOUND_TRANSITION& Rule : m_BoundRules)
	{
		if (Rule.isDisabled)
			continue;
		if (Rule.iAnimGuard != UINT_MAX && m_iCurrentAnimIdx != Rule.iAnimGuard)
			continue;

		_bool isMatch = true;
		for (_uint iSlot : Rule.WhenSlots)
			if (!m_FlagValues[iSlot]) { isMatch = false; break; }
		if (isMatch)
			for (_uint iSlot : Rule.WhenNotSlots)
				if (m_FlagValues[iSlot]) { isMatch = false; break; }
		if (!isMatch)
			continue;

		if (Rule.iNextAnim != UINT_MAX)
		{
			STATE_ENTER_DESC Desc{ Rule.iNextAnim };
			m_pStateMachinCom->Change_State(Rule.Next.iCategory, Rule.Next.iSubState, &Desc);
		}
		else
		{
			m_pStateMachinCom->Change_State(Rule.Next.iCategory, Rule.Next.iSubState);
		}
		return;
	}
}

void CTraceurState::Free()
{
	__super::Free();
}

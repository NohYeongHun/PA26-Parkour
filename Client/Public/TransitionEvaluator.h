#pragma once
#include "Component.h"
#include "StateMachine.h"

namespace Engine { class CAnimationController; class CAnimator; }

NS_BEGIN(Client)

struct BOUND_TRANSITION
{
	vector<vector<_uint>> WhenGroups;
	vector<vector<_uint>> WhenNotGroups;
	_uint            iAnimGuard = UINT_MAX;
	Engine::StateKey Next{ 0, 0 };
	_uint            iNextAnim  = UINT_MAX;
	_float           fBlendOverride = -1.f;
	_bool            isDisabled = false;
};

class CTransitionEvaluator final : public Engine::CComponent
{
public:
	typedef struct tagTransitionEvaluatorDesc : Engine::CComponent::COMPONENT_DESC
	{
	}TRANSITION_EVALUATOR_DESC;

protected:
	explicit CTransitionEvaluator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CTransitionEvaluator(const CTransitionEvaluator& Prototype);
	virtual ~CTransitionEvaluator() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	_bool Evaluate();

private:
	void  Bind_Rules(const class CTransitionTable* pTable);
	void  Bind_One(const _string& strKeyName, const struct TRANSITION_RULE_DATA& Data,
	               vector<BOUND_TRANSITION>& OutBounds, _string& strWarnings);
	_bool Try_Fire(const BOUND_TRANSITION& Rule, _uint iCurrentAnim, const Engine::StateKey& CurKey);

private:
	class CStateBlackboard*           m_pBlackboardCom   = { nullptr };
	Engine::CStateMachine*            m_pStateMachineCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom     = { nullptr };
	class CParkourDeciderComponent*   m_pDeciderCom      = { nullptr };
	Engine::CAnimator*                m_pAnimator        = { nullptr };
	Engine::CAnimationController*     m_pAnimCtrlCom     = { nullptr };

	map<Engine::StateKey, vector<BOUND_TRANSITION>> m_BoundRules;
	map<_uint, vector<BOUND_TRANSITION>>            m_BoundCategoryRules;
	_uint m_iBoundVersion = 0;

public:
	static CTransitionEvaluator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

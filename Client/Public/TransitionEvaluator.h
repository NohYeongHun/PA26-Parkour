#pragma once
#include "Component.h"
#include "StateMachine.h"

namespace Engine { class CAnimationController; }

NS_BEGIN(Client)

struct BOUND_TRANSITION
{
	vector<_uint>    WhenSlots;
	vector<_uint>    WhenNotSlots;
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
	void Bind_Rules(const class CTransitionTable* pTable);

private:
	class CStateBlackboard*           m_pBlackboardCom   = { nullptr };
	Engine::CStateMachine*            m_pStateMachineCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom     = { nullptr };
	class CParkourDeciderComponent*   m_pDeciderCom      = { nullptr };
	class CModel*                     m_pModelCom        = { nullptr };
	Engine::CAnimationController*     m_pAnimCtrlCom     = { nullptr };

	map<Engine::StateKey, vector<BOUND_TRANSITION>> m_BoundRules;
	_uint m_iBoundVersion = 0;

public:
	static CTransitionEvaluator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

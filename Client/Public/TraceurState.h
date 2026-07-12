#pragma once
#include "State.h"
#include "StateMachine.h"
#include "Client_Struct.h"

NS_BEGIN(Client)

struct TransitionRule
{
	function<_bool()>              Condition;
	Engine::StateKey               Next;
	_uint                          iNextAnim = UINT_MAX;
	shared_ptr<STATE_ENTER_DESC>   pDesc     = nullptr;
};

class CTraceurState abstract : public Engine::CState
{
protected:
	explicit CTraceurState() = default;
	virtual ~CTraceurState() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

public:
	_bool IsVault() const;
	virtual _bool Play_Animation(_float fTimeDelta);

public:
	void Add_Transition(function<_bool()> Condition, Engine::StateKey Next, _uint iNextAnim = UINT_MAX);
	void Add_Transition(function<_bool()> Condition, Engine::StateKey Next, shared_ptr<STATE_ENTER_DESC> pDesc);

protected:
	virtual void Check_State() {}
	virtual void Update_Animations(_float fTimeDelta) {}
	virtual void Check_Physics(_float fTimeDelta) {}
	virtual void Late_Anim_Update(_float fTimeDelta) {}
	virtual void State_Reset() {}
	virtual void SetUp_Animations() = 0;
	virtual void SetUp_Transitions() {}

private:
	void Evaluate_Transitions();
	vector<TransitionRule> m_Transitions;

protected:
	class CTraceur*                    m_pOwner              = { nullptr };
	class CModel*                      m_pModelCom           = { nullptr };
	class CTransform*                  m_pTransformCom       = { nullptr };
	class CCollider*                   m_pColliderCom        = { nullptr };
	class CStateMachine*               m_pStateMachinCom     = { nullptr };
	class CInputController*            m_pInputControllerCom = { nullptr };
	class CEnvironmentQueryComponent*  m_pEnvQueryCom        = { nullptr };
	class CMovementComponent*          m_pMoveCom            = { nullptr };

protected:
	_uint m_iMoveKey = {};

public:
	virtual void Free() override;
};

NS_END

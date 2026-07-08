#pragma once
#include "State.h"

NS_BEGIN(Client)
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
	//_bool IsState(EStateCategory eCategory);
	_bool IsVault() const;


public:
	virtual _bool Play_Animation(_float fTimeDelta);

protected:
	class CTraceur* m_pOwner = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };
	class CCollider* m_pColliderCom = { nullptr };
	class CStateMachine* m_pStateMachinCom = { nullptr };
	class CInputController* m_pInputControllerCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom = { nullptr };
	class CMovementComponent* m_pMoveCom = { nullptr };

protected:
	virtual void Check_StateTransition(_float fTimeDelta) = 0;
	virtual void SetUp_Animations() = 0;

protected:
	_uint m_iMoveKey = {};

public:
	virtual void Free() override;
};

NS_END

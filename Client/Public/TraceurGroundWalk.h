#pragma once
#include "GroundState.h"
NS_BEGIN(Client)
class CTraceurGroundWalk final : public CGroundState
{
private:
	enum STATE
	{
		MOVE = 0,
		RUN,
		VAULT,
		END
	};

public:
	explicit CTraceurGroundWalk() = default;
	virtual ~CTraceurGroundWalk() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	// Update
	void Check_State();
	void Update_Animations(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);


private:
	virtual void Check_StateTransition(_float fTimeDelta) override;
	virtual void SetUp_Animations() override;
	
private:
	void State_Reset();

private:
	_bool m_States[STATE::END];

public:
	static CTraceurGroundWalk* Create(class CTraceur* pOwner);
	virtual void Free() override;

};
NS_END


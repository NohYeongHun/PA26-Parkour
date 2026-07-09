#pragma once
#include "AirState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurAirFall final : public CAirState
{
private:
	enum STATE
	{
		END
	};

public:
	explicit CTraceurAirFall() = default;
	virtual ~CTraceurAirFall() = default;

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

public:
	static CTraceurAirFall* Create(class CTraceur* pOwner);
	virtual void Free() override;

};
NS_END

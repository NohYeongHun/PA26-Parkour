#pragma once
#include "AirState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurAirFall final : public CAirState
{
public:
	enum STATE
	{
		LAND,
		END
	};


public:
	explicit CTraceurAirFall() = default;
	virtual ~CTraceurAirFall() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;

private:
	void Check_State() override;
	void State_Reset() override;

private:
	virtual void SetUp_Animations() override;
	virtual void SetUp_Transitions() override;

private:
	_bool m_States[STATE::END];

public:
	static CTraceurAirFall* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

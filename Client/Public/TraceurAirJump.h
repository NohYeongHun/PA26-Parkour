#pragma once
#include "AirState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurAirJump final : public CAirState
{
private:
	enum STATE
	{
		LAND,
		RUN,
		MOVE,
		END
	};

public:
	explicit CTraceurAirJump() = default;
	virtual ~CTraceurAirJump() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Check_State() override;
	void Update_Animations(_float fTimeDelta) override;
	void Check_Physics(_float fTimeDelta) override;
	void State_Reset() override;

private:
	virtual void SetUp_Animations() override;
	virtual void SetUp_Transitions() override;

private:
	_bool  m_States[STATE::END] = {};
	_float m_fVelocityY =		{};

public:
	static CTraceurAirJump* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

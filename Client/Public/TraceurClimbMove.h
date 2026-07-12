#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbMove final : public CClimbState
{
private:
	enum STATE
	{
		JUMP,
		LAND,
		FALL,
		KNEE_HIT,
		END
	};

public:
	explicit CTraceurClimbMove() = default;
	virtual ~CTraceurClimbMove() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Check_State() override;
	void Update_Animations(_float fTimeDelta) override;
	void State_Reset() override;

private:
	virtual void SetUp_Animations() override;
	virtual void SetUp_Transitions() override;

private:
	_bool m_States[STATE::END] = {};

public:
	static CTraceurClimbMove* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

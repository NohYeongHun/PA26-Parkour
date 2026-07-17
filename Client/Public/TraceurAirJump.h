#pragma once
#include "AirState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurAirJump final : public CAirState
{
public:
	explicit CTraceurAirJump() = default;
	virtual ~CTraceurAirJump() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;
	void Check_Physics(_float fTimeDelta) override;

public:
	static CTraceurAirJump* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

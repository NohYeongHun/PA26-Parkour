#pragma once
#include "AirState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurAirFall final : public CAirState
{
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

private:
	virtual void SetUp_Animations() override;

public:
	static CTraceurAirFall* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

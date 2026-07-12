#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbMantle final : public CClimbState
{
public:
	explicit CTraceurClimbMantle() = default;
	virtual ~CTraceurClimbMantle() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;

public:
	static CTraceurClimbMantle* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

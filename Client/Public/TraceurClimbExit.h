#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbExit final : public CClimbState
{
public:
	explicit CTraceurClimbExit() = default;
	virtual ~CTraceurClimbExit() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;
	virtual void SetUp_Transitions() override;

public:
	static CTraceurClimbExit* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

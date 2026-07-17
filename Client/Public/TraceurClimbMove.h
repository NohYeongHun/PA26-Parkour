#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbMove final : public CClimbState
{
public:
	explicit CTraceurClimbMove() = default;
	virtual ~CTraceurClimbMove() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;

public:
	static CTraceurClimbMove* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

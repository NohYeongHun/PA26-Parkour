#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbRun final : public CClimbState
{
public:
	explicit CTraceurClimbRun() = default;
	virtual ~CTraceurClimbRun() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Check_State() override;
	void Update_Animations(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;
	_bool Ready_WallRun();

private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};

public:
	static CTraceurClimbRun* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

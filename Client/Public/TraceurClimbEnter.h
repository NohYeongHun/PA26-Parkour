#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbEnter final : public CClimbState
{
private:
	enum STATE
	{
		END
	};

public:
	explicit CTraceurClimbEnter() = default;
	virtual ~CTraceurClimbEnter() = default;

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


private:
	_bool      m_bValidPlan = false;
	ENV_QUERY_RESULT m_EnvQueryResult = {};

private:
	// Vault 비행 커브 (2차 Bezier)
	_float3 m_vCurveP0 = {};
	_float3 m_vCurveP1 = {};
	_float3 m_vCurveP2 = {};
	_float m_fCurveT = {};
	_bool   m_bValidCurve = false;

public:
	static CTraceurClimbEnter* Create(class CTraceur* pOwner);
	virtual void Free() override;

};
NS_END

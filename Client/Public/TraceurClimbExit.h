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
	void Late_Anim_Update(_float fTimeDelta) override;
	void Check_State() override;

private:
	virtual void SetUp_Animations() override;

#ifdef _DEBUG
private:
	void Draw_DebugCurve();
#endif

private:
	void  Move_AlongCurve(_float fTimeDelta);

private:
	void Build_Curve();
	void End_Traversal();



private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};
	_float3 m_vCurveP0 = {};
	_float3 m_vCurveP1 = {};
	_float3 m_vCurveP2 = {};
	_float  m_fCurveT = {};
	_bool   m_bValidCurve = false;
	_bool   m_isMantle = false;
	_bool   m_isWarpBegun = false;       

public:
	static CTraceurClimbExit* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

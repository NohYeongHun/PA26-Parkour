#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbEnter final : public CClimbState
{
public:
	explicit CTraceurClimbEnter() = default;
	virtual ~CTraceurClimbEnter() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;
	void Late_Anim_Update(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;
	virtual void SetUp_Transitions() override;

private:
	_bool  Ready_Enter();
	_bool  Select_Animation();
	void   Build_Curve();

#ifdef _DEBUG
private:
	void Draw_DebugCurve();
#endif

private:
	void Move_AlongCurve(_float fTimeDelta);

#ifdef _DEBUG
private:
	_float3 m_vDebugWallNormal = {};
	_float3 m_vDebugWallHitPos = {};
	_float3 m_vDebugWallEndPos = {};
#endif

private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};
	_float3 m_vCurveP0    = {};
	_float3 m_vCurveP1    = {};
	_float3 m_vCurveP2    = {};
	_float3 m_vLookTarget = {};
	_float3 m_vLookStart  = {};
	_float  m_fCurveT     = {};
	_bool   m_isValidCurve = {};

public:
	static CTraceurClimbEnter* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

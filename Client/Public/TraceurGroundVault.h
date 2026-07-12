#pragma once
#include "GroundState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurGroundVault final : public CGroundState
{
public:
	explicit CTraceurGroundVault() = default;
	virtual ~CTraceurGroundVault() = default;

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
	void   Build_Curve();

#ifdef _DEBUG
private:
	void Draw_DebugCurve();
#endif

private:
	void  Move_AlongCurve(_float fTimeDelta);
	_bool Select_Animation();

private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};
	_float3 m_vCurveP0    = {};
	_float3 m_vCurveP1    = {};
	_float3 m_vCurveP2    = {};
	_float  m_fCurveT     = {};
	_bool   m_bValidCurve = false;

public:
	static CTraceurGroundVault* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

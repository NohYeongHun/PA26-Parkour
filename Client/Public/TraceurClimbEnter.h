#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
/* Climb 목표 위치에 스냅을 위한 애니메이션 재생  */
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
	

private:
	void State_Reset();

private:
	_bool Ready_Enter();
	_bool Select_Animation();
	void  Build_Curve();

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
#endif // _DEBUG


	


private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};
	// Vault 비행 커브 (2차 Bezier)
	_float3 m_vCurveP0 = {};
	_float3 m_vCurveP1 = {};
	_float3 m_vCurveP2 = {};
	
	_float3 m_vLookTarget = {};
	_float3 m_vLookStart = {};


	_float m_fCurveT = {};
	_bool m_isValidCurve = {};


private:
	virtual void SetUp_Animations() override;

public:
	static CTraceurClimbEnter* Create(class CTraceur* pOwner);
	virtual void Free() override;

};
NS_END

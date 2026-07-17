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
	_bool  Ready_Enter();
	_bool Select_Animation();

private:
#ifdef _DEBUG
	_float3 m_vCurveP0 = {};
	_float3 m_vCurveP1 = {};
	_float3 m_vCurveP2 = {};
	_float  m_fCurveT = {};
#endif // _DEBUG


#ifdef _DEBUG
private:
	void Draw_Debug();
#endif // _DEBUG



	

public:
	static CTraceurGroundVault* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

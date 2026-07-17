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
	_bool  Ready_Enter();
	_bool  Select_Animation();

#ifdef _DEBUG
private:
	_float3 m_vDebugWallNormal = {};
	_float3 m_vDebugWallHitPos = {};
	_float3 m_vDebugWallEndPos = {};
#endif

public:
	static CTraceurClimbEnter* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

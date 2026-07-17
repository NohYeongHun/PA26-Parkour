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
	void Update_Animations(_float fTimeDelta) override;

private:
	_bool Ready_WallRun(void* pArg);

#ifdef _DEBUG
private:
	void Draw_DebugSteer();

private:
	_float3 m_vDebugWallNormal = {}; // 월드 공간 벽 노멀
	_float3 m_vDebugAxisWorld  = {}; // 월드 공간 틸트축
	_float3 m_vDebugAxisLocal  = {}; // 로컬 공간 틸트축 (진입 시점 값 고정)
	_bool   m_hasDebugSteer    = false;
#endif

private:
	static constexpr _float FTILT_ANGLE_DEG = 65.f;
	static constexpr _float FWALL_SNAP_DIST = 0.2f;
	static constexpr _float FBLEND_IN_TIME  = 0.25f;
	static constexpr _float FBLEND_OUT_TIME = 0.2f;
public:
	static CTraceurClimbRun* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

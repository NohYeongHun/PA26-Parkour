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
	static constexpr _float FBLEND_IN_TIME  = 0.25f; // Mesh 좌표계 블렌딩할 시간
	static constexpr _float FBLEND_OUT_TIME = 0.2f;
	static constexpr _float FLIFTOFF_TIMEOUT = 0.4f;
	static constexpr _float FTOP_ARRIVE_MARGIN = 0.05f; // 발이 상단면 높이에 이만큼 못 미쳐도 도착 인정

private:
	ENV_QUERY_RESULT m_EnvQueryResult = {};
	_float3 m_vClimbNormal = {};
	_bool m_hasLeftGround = false; // 이륙 확인 전에는 Land 플래그 무시 (진입 직후 즉시 이탈 방지)
	_float m_fGroundedTime = 0.f;  // 이륙 실패 누적 시간 — FLIFTOFF_TIMEOUT 초과 시 지상 복귀

	// 상단 도착용 캐시 — 발이 상단을 넘으면 레이가 전부 빗나가 Geo가 비므로, 보이는 동안 저장해둔다
	_float3 m_vTopStandCache = {};
	_bool   m_hasTopStandCache = false;
public:
	static CTraceurClimbRun* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END

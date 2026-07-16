#pragma once
#include "Component.h"

NS_BEGIN(Client)
/*
* 메시의 물리적 형태가 어떤 파쿠르 동작을 허용할지를 판단하는 컴포넌트.
*/

enum class HEIGHT_HIT_FLAG {
	NONE = 0,
	KNEE = 1 << 0, // 무릎 높이
	CHEST = 1 << 1, // 가슴 높이
	HEAD = 1 << 2, // 머리 높이
	ALL = 0x7,
	END
};

class CEnvironmentQueryComponent final : public Engine::CComponent
{
public:
	typedef struct tagEnvQueryComponentDesc : Engine::CComponent::COMPONENT_DESC
	{
		_float fShapeTraceDistance = 2.f;
		_float fLineTraceDistance = 2.f;
		COLLISIONLAYER eTargetLayer = COLLISIONLAYER::END;
	}ENV_QUERY_DESC;

private:
	static constexpr _float FHEAD_RATIO		 = 1.1f;
	static constexpr _float FCHEST_RATIO	 = 0.8f;
	static constexpr _float FKNEE_RATIO		 = 0.35f;
	static constexpr _float FMAX_REACH_RATIO = 1.5f;

	// 두께 탐지
	static constexpr _uint  FDEPTH_SAMPLE_COUNT    = 3;
	static constexpr _uint  FEDGE_REFINE_ITERATIONS = 4; // 뒷모서리 이진 탐색 횟수 (오차 = 샘플 간격 / 2^4)
	static constexpr _float FLANDING_MAX_HEIGHT_DIFF = 0.3f; // 착지점 주변 검증 레이의 허용 높이 차

	// 액션별 판정 임계값 — 이후 JSON 튜닝 테이블로 이관 가능한 형태로 묶는다
	struct VAULT_THRESHOLDS {
		_float fMinApproachDot       = 0.9f;
		_float fHighVaultHeightRatio = 0.8f; // 전고 배율 — 이상이면 HIGH_VAULT (기존 가슴 레이 높이와 동일)
	};
	struct MANTLE_THRESHOLDS {
		_float fMinDepthMult   = 1.0f; // 반지름 배율 — 최소 상단 두께
		_float fMinWidthMult   = 2.0f; // 반지름 배율 — 최소 상단 폭 (Task 6에서 적용)
		_float fMinApproachDot = 0.5f;
	};
	struct CLIMB_THRESHOLDS {
		_float fMaxHeightRatio = 1.5f; // 전고 배율 — 모서리 도달 가능 최대 높이
	};
	struct WALLRUN_THRESHOLDS {
		_float fMinApproachDot   = 0.85f; // 정면 접근 판정 (Vault 0.9보다 관대)
		_float fMaxNormalY       = 0.1f;  // 벽면 수직성 — 전면 노멀 Y 성분 허용치
		_float fMaxStartDistMult = 1.1f;  // 반지름 배율 — 벽 부착 판정 거리 (이내여야 시작)
	};
	static constexpr VAULT_THRESHOLDS  VAULT_TH{};
	static constexpr MANTLE_THRESHOLDS MANTLE_TH{};
	static constexpr CLIMB_THRESHOLDS  CLIMB_TH{};
	static constexpr WALLRUN_THRESHOLDS WALLRUN_TH{};

	// 디버그 레이 색 분류 — Cast_Ray가 시각화에 사용
	enum class RAY_KIND { SCAN, MEASURE, REFINE };

protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	const ENV_QUERY_RESULT& Get_QueryResult() const { return m_EnvQueryResult; }

	// 스캔 기준 방향 오버라이드 — 월런처럼 몸(LOOK)을 눕히는 상태에서 벽 감지를 유지한다.
	// 설정 시 모든 레이/Shape캐스트가 LOOK 대신 이 방향(XZ 정규화) 기준. 상태 이탈 시 Clear 필수.
	void Set_ScanDirOverride(_fvector vDir);
	void Clear_ScanDirOverride() { m_hasScanDirOverride = false; }

public:
	void Execute();
	_bool Find_Ground(const _fvector& vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos);

private:
	_bool Detect_Obstacle();    
	void  Scan_Obstacle();      
	void  Measure_Geometry();   
	void  Judge_Actions();      

	void Judge_TopReaced(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo);

	ACTION_VERDICT Judge_Vault(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	ACTION_VERDICT Judge_Mantle(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	ACTION_VERDICT Judge_Climb(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const;
	ACTION_VERDICT Judge_Hang(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const;
	ACTION_VERDICT Judge_WallRun(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	_uint          Get_ObjectFlagMask(const OBSTACLE_SCAN& Scan) const; // END(태그 없음)는 ALL로 정규화

private:
	LINE_TRACE_HIT Cast_Ray(const _fvector& vStart, const _fvector& vEnd, _uint iLayer, RAY_KIND eKind);
	_vector Get_ScanDir() const; // 오버라이드 또는 Transform LOOK (정규화 보장)

#ifdef _DEBUG
private:
	void Draw_DebugMarkers();
#endif // _DEBUG

#ifdef _DEBUG
public:
	void Print_Debug();
#endif // _DEBUG

private:
	// Owner에게서 참조하는 컴포넌트
	class CTransform* m_pOwnerTransformCom = { nullptr };
	class CCollider* m_pOwnerColliderCom = { nullptr };

private:
	_float m_fShapeTraceDistance = { 2.f };
	_float m_fLineTraceDistance = { 2.f };
	COLLISIONLAYER m_eTargetLayer = { COLLISIONLAYER::END };

private:
	ENV_QUERY_RESULT m_EnvQueryResult{};
	_float3 m_vScanDirOverride = {};
	_bool   m_hasScanDirOverride = false;

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

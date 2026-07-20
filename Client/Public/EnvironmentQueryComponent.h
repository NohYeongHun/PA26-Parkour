#pragma once
#include "Component.h"
#include "Client_Struct.h"

NS_BEGIN(Client)

class CEnvironmentQueryComponent final : public Engine::CComponent
{
public:
	typedef struct tagEnvQueryComponentDesc : Engine::CComponent::COMPONENT_DESC
	{
		_float fShapeTraceDistance = 2.f;
		_float fLineTraceDistance = 2.f;
		COLLISIONLAYER eTargetLayer = COLLISIONLAYER::END;
		const BODY_PROFILE* pBodyProfile = nullptr;
	}ENV_QUERY_DESC;

private:
	// 두께 탐지
	static constexpr _uint  FDEPTH_SAMPLE_COUNT    = 3;
	static constexpr _uint  FEDGE_REFINE_ITERATIONS = 4;
	static constexpr _float FLANDING_MAX_HEIGHT_DIFF = 0.3f;

	// 디버그 레이 색 분류
	enum class RAY_KIND { SCAN, MEASURE, REFINE };

	struct MEASURE_FRAME
	{
		_vector vBottom{};
		_vector vTraversal{};
		_vector vStartXZ{};
		_float  fStartY = 0.f;
		_float  fTopSurfaceY = 0.f;
		_float  fRadius = 0.f;
		_float  fTotalHeight = 0.f;
	};

protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	const ENV_PERCEPTION& Get_Perception() const { return m_Perception; }

	void Set_ScanDirOverride(_fvector vDir);
	void Clear_ScanDirOverride() { m_hasScanDirOverride = false; }

public:
	void Execute();
	_bool Find_Ground(const _fvector& vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos);

private:
	_bool Detect_Obstacle();
	void  Scan_Obstacle();
	void  Scan_Reach();
	void  Probe_ReachEdge(const _fvector& vBottom);

private:
	void   Measure_Geometry();
	MEASURE_FRAME Make_MeasureFrame() const;
	void   Measure_Front(MEASURE_FRAME& Frame);
	_bool  Measure_Top(MEASURE_FRAME& Frame);
	void   Measure_TopWidth(const MEASURE_FRAME& Frame);
	void   Measure_Depth(const MEASURE_FRAME& Frame);
	_float Refine_DepthEdge(const MEASURE_FRAME& Frame, _float fLo, _float fHi);
	void   Measure_StandPos(const MEASURE_FRAME& Frame);
	void   Measure_Landing(const MEASURE_FRAME& Frame);

private:
	LINE_TRACE_HIT Cast_Ray(const _fvector& vStart, const _fvector& vEnd, _uint iLayer, RAY_KIND eKind);
	LINE_TRACE_HIT Cast_Ray_WithMapFallback(const _fvector& vStart, const _fvector& vEnd, RAY_KIND eKind);
	_vector Get_ScanDir() const;

#ifdef _DEBUG
private:
	void Draw_DebugMarkers();
	void Log_ShapeHit(const SHAPE_CAST_HIT& Hit);
#endif

#ifdef _DEBUG
public:
	void Print_Debug();

private:
	const void* m_pDebugLastShapeHitDesc = nullptr; // ShapeCast 히트 오브젝트 변경 시에만 로그 (dedup)
#endif

private:
	class CTransform* m_pOwnerTransformCom = { nullptr };
	class CCollider* m_pOwnerColliderCom = { nullptr };
	const BODY_PROFILE* m_pBodyProfile = nullptr;

private:
	_float m_fShapeTraceDistance = { 2.f };
	_float m_fLineTraceDistance = { 2.f };
	COLLISIONLAYER m_eTargetLayer = { COLLISIONLAYER::END };

private:
	ENV_PERCEPTION m_Perception{};
	_float3 m_vScanDirOverride = {};
	_bool   m_hasScanDirOverride = false;

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

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

	typedef struct tagLineTraceHit
	{
		_bool		isHit = { false };
		_float      fCenterDistance = 0.f;
		_float3     vHitPosition{};
		_float3     vHitNormal{};
	}LINE_TRACE_HIT;

private:
	static constexpr _float FHEAD_RATIO		 = 1.1f;
	static constexpr _float FCHEST_RATIO	 = 0.8f;
	static constexpr _float FKNEE_RATIO		 = 0.35f;
	static constexpr _float FMAX_REACH_RATIO = 1.5f;

	// 두께 탐지
	static constexpr _uint  FDEPTH_SAMPLE_COUNT    = 3;
	static constexpr _float FVAULT_MAX_DEPTH_MULT  = 2.0f; // Vault 가능 최대 두께 (반지름 배율)
	static constexpr _float FMANTLE_MIN_DEPTH_MULT = 1.0f; // Mantle 가능 최소 두께 (반지름 배율)
	static constexpr _float FMAX_CLIMBABLE_HEIGHT_RATIO = FMAX_REACH_RATIO;

protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	const ENV_QUERY_RESULT& Get_QueryResult() const { return m_EnvQueryResult; }

	void Execute();
	_bool Find_Ground(const _fvector& vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos);

private:
	_bool Detect_Obstacle();
	void Collect_RayInfo();
	void Classify_HitFlag();
	void Extract_Geometry();
	void Judge_Condition();

	LINE_TRACE_HIT Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos);

#ifdef _DEBUG
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
	// ShapeCast로 탐지한 Object의 태그.
	PARKOUR_FLAG m_eObjectParkourFlag = { PARKOUR_FLAG::END };

	LINE_TRACE_HIT m_KneeHit{};
	LINE_TRACE_HIT m_ChestHit{};
	LINE_TRACE_HIT m_HeadHit{};

	// 수평 레이 Hit 패턴
	_uint m_iHeightFlag = {};
	ENV_QUERY_RESULT m_EnvQueryResult{};

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

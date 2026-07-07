#pragma once
#include "Component.h"

NS_BEGIN(Client)
/*
* 메시의 물리적 형태가 어떤 파쿠르 동작을 허용할지를 판단하는 컴포넌트.
*/

enum class HEIGHT_HIT_FLAG {
	NONE = 0,
	KNEE = 1 << 0, // 무릎 이상
	CHEST = 1 << 1, // 
	HEAD = 1 << 2,
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
	static constexpr _float FHEAD_RATIO		 = 1.1f; // 머리
	static constexpr _float FCHEST_RATIO	 = 0.8f; // 가슴팍
	static constexpr _float FKNEE_RATIO		 = 0.35f;  // 무릎
	static constexpr _float FMAX_REACH_RATIO = 1.5f;  // 손 뻗은 최대 높이.

protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	const ENV_QUERY_RESULT& Get_QueryResult() const { return m_EnvQueryResult; }

public:
	void Change_TargetLayer(COLLISIONLAYER eLayer) { m_eTargetLayer = eLayer; }
	

public:
	void Execute(_float fTimeDelta);
	_bool Detect_Obstacle(_float fTimeDelta);
	void Collect_RayInfo(_float fTimeDelta);

private:
	// Component => 참조할
	class CTransform* m_pOwnerTransformCom = { nullptr };
	class CCollider* m_pOwnerColliderCom = { nullptr };

private:
	_float m_fShapeTraceDistance = { 4.f };
	_float m_fLineTraceDistance = { 4.f };
	COLLISIONLAYER m_eTargetLayer = { COLLISIONLAYER::END };

private:
	LINE_TRACE_HIT m_KneeHit{}; // 무릎 부분 Hit 체크
	LINE_TRACE_HIT m_ChestHit{};
	LINE_TRACE_HIT m_HeadHit{};

private:
	ENV_QUERY_RESULT m_EnvQueryResult{};

#ifdef _DEBUG
private:
	void Print_Debug();
#endif // _DEBUG


private:
	_uint m_iHeightFlag = {};



private:
	SHAPE_CAST_HIT m_OutHit{};

private:
	// ShapeCast로 탐지한 Object의 태그.
	PARKOUR_FLAG m_eObjectParkourFlag = { PARKOUR_FLAG::END };
	// 이번 프레임에 유효한 후보군
	_uint m_iCandidateFlag = {};

private:
	LINE_TRACE_HIT Ray_Cast(const _fvector& vStartPos, const _fvector& vDir);
	void Classify_HitFlag();
	void Start_DownRayCast();
	void Judge_Condition();

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


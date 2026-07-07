#pragma once
#include "Component.h"

NS_BEGIN(Client)
/*
* 메시의 물리적 형태가 어떤 파쿠르 동작을 허용할지를 판단하는 컴포넌트.
*/
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
		_bool   isHit = { false };
		_float4 vHitPoint = {};
	}LINE_TRACE_HIT;

protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	void Change_TargetLayer(COLLISIONLAYER eLayer) { m_eTargetLayer = eLayer; }

public:
	void Execute(_float fTimeDelta);
	_bool Detect_Obstacle(_float fTimeDelta);
	void Detect_ObstacleShape(_float fTimeDelta);

private:
	// Component => 참조할
	class CTransform* m_pOwnerTransformCom = { nullptr };
	class CCollider* m_pOwnerColliderCom = { nullptr };

private:
	_float m_fShapeTraceDistance = { 4.f };
	_float m_fLineTraceDistance = { 4.f };
	COLLISIONLAYER m_eTargetLayer = { COLLISIONLAYER::END };

private:
	LINE_TRACE_HIT m_BottomHit{};
	LINE_TRACE_HIT m_ChestHit{};
	LINE_TRACE_HIT m_HeadHit{};

private:
	static constexpr _float FHEAD_RATIO = 0.9f;   // 머리
	static constexpr _float FCHEST_RATIO = 0.55f; // 가슴팍

private:
	SHAPE_CAST_HIT m_OutHit{};

private:
	// 탐지 정보. => Struct 예정.
	PARKOUR_FLAG m_eParkourFlag = { PARKOUR_FLAG::END };
	

private:
	LINE_TRACE_HIT Ray_Cast(const _fvector& vStartPos, const _fvector& vDir);

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


#pragma once
#include "CollideComponent.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCollider final : public CCollideComponent
{
public:
	typedef struct tagColliderDesc {
		_float3			vPos;
		_float4			vQuat = _float4(0.f, 0.f, 0.f, 1.f);
		_float3			vOffset;
		_float				fRayOffset = { -0.2f };
		EMotionType	eType;
		_uint				iLayer;
		_float				fHeight;		// Capsule Height
		_float				fRadius;		// Capsule Radius
	}COLLIDER_DESC;

private:
	explicit CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;

public:
	// Data (void*)
	void								Set_Desc(void* pData) { m_tCollisionData.pDesc = pData; }
	// Collider 움직임 -> Transform에 적용
	void								Sync_Position(class CTransform* pTransform);
	// 땅을 타고 있는지 Check
	_bool								IsLand(_float3* pNormalOut = nullptr);
	// Gravity On/Off
	void								Set_Gravity(_bool isGravity) { m_isGravity = isGravity; }
	// Set Pos
	void								Set_Position(const _fvector& vPos) { m_pCharacterVirtual->SetPosition(LoadVec3(vPos)); }

	_vector							Get_Position() const
	{
		Vec3 vPos = m_pCharacterVirtual->GetPosition();
		return XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f);
	}

	void								Set_Offset(const _float3 vOffset);

	void								IsActivate(_bool isActive);
	void								Change_Layer(_uint iLayer);

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	void								Update(const _fvector& vVelocity);
	virtual		HRESULT				Render() override;

private:
	Ref<CharacterVirtual>		m_pCharacterVirtual = { nullptr };
	COLLISION_DATA				m_tCollisionData = {};
	BodyInterface*					m_pBodyInterface = { nullptr };
	BodyID							m_BodyID = {};

	_bool								m_isActivate = { true };

	_uint								m_iCollisionLayer = {};

	_bool								m_isGravity = { true };

	_float3							m_vOffset = {};
	_float								m_fRayOffset = {};
	RefConst<Shape>				m_pShape = { nullptr };

	_bool								m_isLand = { true };
private:
	// 경사로에서 이동속도 변화 -> Slide로 보정
	Vec3								Slide(const Vec3& Velocity);

public:
	static		CCollider*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END
#include "EnginePch.h"
#include "Collider.h"
#include "GameInstance.h"

#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollideComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CCollideComponent{ Prototype }
{
}

void CCollider::Sync_Position(CTransform* pTransform)
{
	Vec3 vPos = m_pCharacterVirtual->GetPosition();

	//_vector vLerpPos = XMVectorLerp(pTransform->Get_State(STATE::POSITION), XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f), 0.15f);
	_vector vLerpPos = XMVectorLerp(pTransform->Get_State(STATE::POSITION), XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f), 0.3f);

	//pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
	pTransform->Set_State(STATE::POSITION, XMVectorSetW(vLerpPos, 1.f));
}

_bool CCollider::IsLand(_float3* pNormalOut)
{
	if (nullptr == m_pCharacterVirtual)
		return false;

	if (nullptr != pNormalOut)
		*pNormalOut = StoreFloat3(m_pCharacterVirtual->GetGroundNormal());

	return m_pCharacterVirtual->IsSupported();
	//return m_isLand;
}

void CCollider::Set_Offset(const _float3 vOffset)
{
	m_vOffset = vOffset;

	if (m_pCharacterVirtual)
	{
		// Jolt CharacterVirtual의 ShapeOffset을 실시간 업데이트
		m_pCharacterVirtual->SetShapeOffset(LoadVec3(m_vOffset));
	}
}

void CCollider::IsActivate(_bool isActive)
{
	if (false == m_isActivate && true == isActive)
	{
		m_pBodyInterface->AddBody(m_BodyID, EActivation::Activate);
		m_pGameInstance->Register_Virtual(m_pCharacterVirtual);
		m_isActivate = true;
	}
	else if (true == m_isActivate && false == isActive)
	{
		m_pBodyInterface->RemoveBody(m_BodyID);
		m_pGameInstance->Remove_Virtual(m_pCharacterVirtual);
		m_isActivate = false;
	}

	//m_isActivate = isActive;
}

void CCollider::Change_Layer(_uint iLayer)
{
	if (true == m_isActivate)
		m_pBodyInterface->SetObjectLayer(m_BodyID, ObjectLayer(iLayer));
}

HRESULT CCollider::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CCollider::Initialize_Clone(void* pArg)
{
	m_isClone = true;
	ASSERT_CRASH(pArg);

	COLLIDER_DESC* pDesc = static_cast<COLLIDER_DESC*>(pArg);
	m_iCollisionLayer = pDesc->iLayer;

	// Create Shape
	using namespace JPH;
	m_fRayOffset = pDesc->fRayOffset;
	m_pShape = new CapsuleShape(pDesc->fHeight * 0.5f, pDesc->fRadius);
	ASSERT_CRASH(m_pShape);

	m_vOffset = pDesc->vOffset;
	// Virtual Setting
	CharacterVirtualSettings VirtualSetting = {};
	VirtualSetting.mMaxSlopeAngle = XMConvertToRadians(100.f);
	//VirtualSetting.mMaxSlopeAngle = XMConvertToRadians(50.f);			// 허용 경사 각도
	VirtualSetting.mShape = m_pShape;											// Character Virtual Shape
	VirtualSetting.mShapeOffset = LoadVec3(m_vOffset);						// Shape Offset
	VirtualSetting.mMaxStrength = 8.f;											// 다른 Body를 밀 수 있는 최대 힘
	VirtualSetting.mCharacterPadding = 0.015f; //0.02f;									// (충돌 범위 Padding) => 여유 주는듯?
	VirtualSetting.mPenetrationRecoverySpeed = 6.0f; //0.f;							// 겹쳤을 때 복원 속도
	VirtualSetting.mPredictiveContactDistance = 0.08f; //0.02f;							// 미리 충돌 감지하는 범위
	VirtualSetting.mEnhancedInternalEdgeRemoval = true;					// 각진 부분 부드럽게
	
	VirtualSetting.mInnerBodyShape = m_pShape;
	VirtualSetting.mInnerBodyLayer = ObjectLayer(pDesc->iLayer);

	// Create CharacterVirtual
	m_tCollisionData.pComponent = this;
	m_pCharacterVirtual = m_pGameInstance->Register_Virtual(VirtualSetting, LoadVec3(pDesc->vPos), LoadQuat(pDesc->vQuat), &m_tCollisionData, &m_pBodyInterface);
	ASSERT_CRASH(m_pCharacterVirtual);

	m_BodyID = m_pCharacterVirtual->GetInnerBodyID();

    return S_OK;
}

void CCollider::Update(const _fvector& vVelocity)
{
	// Ray Cast Ground Check
	_vector vPos = StoreVector3(m_pCharacterVirtual->GetPosition()) + XMVectorSet(0.f, 0.1f, 0.f, 0.f);
	_vector vEndPos = vPos + XMVectorSet(0.f, m_fRayOffset, 0.f, 0.f);

	_float4 vOut = {};
	m_isLand = m_pGameInstance->Ray_Cast(vPos, vEndPos, &vOut);

	Vec3 Velocity = LoadVec3(vVelocity);
	if (false == m_isLand && true == m_isGravity)
		Velocity += XMVectorSet(0.f, -9.81f, 0.f, 0.f) * 0.7f;
	else
	{
		Slide(Velocity);
		//if (Velocity.GetY() < 0.f)
		//	Velocity.SetY(0.f);
	}

	// Ground Check Error
	CharacterVirtual::EGroundState GS = m_pCharacterVirtual->GetGroundState();
	if ((false == m_isLand && CharacterVirtual::EGroundState::OnGround == GS) || (true == m_isLand && CharacterVirtual::EGroundState::OnGround != GS))
		Velocity += XMVectorSet(0.f, -0.01f, 0.f, 0.f);

	m_pCharacterVirtual->SetLinearVelocity(Velocity);
	m_pGameInstance->Add_Virtual(m_pCharacterVirtual, m_iCollisionLayer);
}

HRESULT CCollider::Render()
{
#ifdef _DEBUG
	if (nullptr != m_pShape)
	{
		RMat44 Matrix = RMat44::sIdentity();
		Vec3 vPos = m_pCharacterVirtual->GetPosition() + m_pCharacterVirtual->GetShapeOffset();
		Matrix.SetColumn4(3, Vec4(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
		m_pGameInstance->DrawShape(m_pShape, Matrix);
	}
#endif
    return S_OK;
}

Vec3 CCollider::Slide(const Vec3& Velocity)
{
	_vector vGroundNormal = XMVector3Normalize(StoreVector3(m_pCharacterVirtual->GetGroundNormal()));

	_float fDot = XMVectorGetX(XMVector3Dot(XMVectorSet(0.f, 1.f, 0.f, 0.f), vGroundNormal));
	if (fDot < cosf(XMConvertToRadians(70.f)))
		return Velocity;

	_vector vVelocity = StoreVector3(Velocity);
	
	_float fLength = XMVectorGetX(XMVector3Dot(vVelocity, vGroundNormal));

	_vector vSlide = vVelocity + -1.f * vGroundNormal * fLength;

	return LoadVec3(vSlide);
}

CCollider* CCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCollider* pInstance = new CCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Collider");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CCollider::Clone(void* pArg)
{
	CCollider* pClone = new CCollider(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Collider (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CCollider::Free()
{
	if (true == m_isClone)
	{
		if (true == m_isActivate)
			m_pGameInstance->Remove_Virtual(m_pCharacterVirtual);
		
		if (false == m_isActivate)
			m_pBodyInterface->AddBody(m_BodyID, EActivation::DontActivate);
		m_pCharacterVirtual = nullptr;
		m_pShape = nullptr;
	}

	__super::Free();

	m_tCollisionData.pComponent = nullptr;
	m_tCollisionData.pDesc = nullptr;
}

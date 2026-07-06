#include "EnginePch.h"
#include "Rigidbody.h"

#include "GameInstance.h"
#include "GameObject.h"

#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"

CRigidbody::CRigidbody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCollideComponent { pDevice, pContext }
{
}

CRigidbody::CRigidbody(const CRigidbody& Prototype)
	: CCollideComponent{ Prototype }
{
}

void CRigidbody::Set_Position(const _fvector& vPos)
{
	m_pBodyInterface->SetPosition(m_BodyID, LoadVec3(vPos), EActivation::Activate);
}

void CRigidbody::Set_Transform(const _fmatrix& Matrix)
{
	_vector vScale{}, vRotation{}, vTranslation{};

	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, Matrix);

	m_pBodyInterface->SetPositionAndRotation(m_BodyID, LoadVec3(vTranslation), LoadQuat(vRotation), EActivation::Activate);
}

HRESULT CRigidbody::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRigidbody::Initialize_Clone(void* pArg)
{
	m_isClone = true;
	ASSERT_CRASH(pArg);

	RIGIDBODY_DESC* pDesc = static_cast<RIGIDBODY_DESC*>(pArg);
	m_eShape = pDesc->eShape;
	RefConst<Shape> BodyShape = { nullptr };

	using namespace JPH;
	switch (m_eShape)
	{
	case SHAPE::SPHERE:
	{
		SPHEREBODY_DESC* pSphereDesc = static_cast<SPHEREBODY_DESC*>(pDesc);
		BodyShape = new SphereShape(pSphereDesc->fRadius);
		break;
	}
	case SHAPE::BOX:
	{
		BOXBODY_DESC* pBoxDesc = static_cast<BOXBODY_DESC*>(pDesc);
		BodyShape = new BoxShape(Vec3(pBoxDesc->vExtent.x, pBoxDesc->vExtent.y, pBoxDesc->vExtent.z));
		break;
	}
	case SHAPE::CAPSULE:
	{
		CAPSULEBODY_DESC* pCapsuleDesc = static_cast<CAPSULEBODY_DESC*>(pDesc);
		BodyShape = new CapsuleShape(pCapsuleDesc->fHeight * 0.5f, pCapsuleDesc->fRadius);
		break;
	}
	case SHAPE::CONVEXHULL:
	{
		CONVEXHULLBODY_DESC* pConvexHullDesc = static_cast<CONVEXHULLBODY_DESC*>(pDesc);
		ASSERT_CRASH(pConvexHullDesc->pModel);
		Ref<ConvexHullShapeSettings> ConvexHullSetting = new ConvexHullShapeSettings(ConvertToArrayVec3(pConvexHullDesc->pModel));
		BodyShape = ConvexHullSetting->Create().Get();
		break;
	}
	case SHAPE::MESH:
	{
		Make_MeshShape(pArg);
		return S_OK;
	}
	default:
		CRASH("Shape Error");
	}

	if (BODYTYPE::BODY == pDesc->eBodyType)
		Ready_Body(pDesc, BodyShape);
	else if(BODYTYPE::CHARACTER == pDesc->eBodyType)
		Ready_Character(pDesc, BodyShape);

	return S_OK;
}

HRESULT CRigidbody::Render()
{
#ifdef _DEBUG
	if(nullptr != m_pBody)
		m_pGameInstance->DrawShape(m_pBody->GetShape(),m_pBody->GetWorldTransform());
#endif
	return S_OK;
}

void CRigidbody::Update_Rigidbody(const _fmatrix& Matrix, _float fTimeDelta)
{
	_vector vScale{}, vRotation{}, vTranslation{};

	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, Matrix);

	m_pBodyInterface->MoveKinematic(m_BodyID, LoadVec3(vTranslation), LoadQuat(vRotation), fTimeDelta);
	
	//m_pBodyInterface->SetPosition(m_BodyID, LoadVec3(vTranslation), EActivation::Activate);
}

void CRigidbody::Sync_Rigidbody(CTransform* pTransform)
{
	Vec3 vPos;
	Quat vRotation;
	m_pBodyInterface->GetPositionAndRotation(m_BodyID, vPos, vRotation);

	_vector vQuaternion = XMVectorSet(vRotation.GetX(), vRotation.GetY(), vRotation.GetZ(), vRotation.GetW());
	pTransform->Rotation_Quaternion(vQuaternion);
	pTransform->Set_State(STATE::POSITION, XMVectorSet(vPos.GetX(), vPos.GetY(), vPos.GetZ(), 1.f));
}

void CRigidbody::Change_Layer(_uint iLayer)
{
	if (SHAPE::MESH == m_eShape)
	{
		for(_uint i = 0; i < m_iNumMesh; ++i)
			m_pBodyInterface->SetObjectLayer(m_pMeshBodyIDs[i], ObjectLayer(iLayer));
	}
	else
		m_pBodyInterface->SetObjectLayer(m_BodyID, ObjectLayer(iLayer));
}

_bool CRigidbody::IsLand(_float3* pNormalOut)
{
	if (nullptr == m_pCharacter)
		return false;

	if (nullptr != pNormalOut)
	{
		Vec3 vNormal = m_pCharacter->GetGroundNormal();
		*pNormalOut = _float3(vNormal.GetX(), vNormal.GetY(), vNormal.GetZ());
	}

	return m_pCharacter->IsSupported();
}

void CRigidbody::IsActivate(_bool isActive)
{
	if (false == m_isActive && true == isActive)
		m_pBodyInterface->AddBody(m_BodyID, EActivation::Activate);
	else if (true == m_isActive && false == isActive)
		m_pBodyInterface->RemoveBody(m_BodyID);
	m_isActive = isActive;
	//true == isActive ? m_pBodyInterface->ActivateBody(m_BodyID) : m_pBodyInterface->DeactivateBody(m_BodyID);
}

void CRigidbody::Change_MotionType(EMotionType eType)
{
	m_pBodyInterface->SetMotionType(m_BodyID, eType, EActivation::Activate);
}

//_bool CRigidbody::Box_Cast(const _fvector& vStartPos, const _fvector& vDir, _float fDistance, vector<BOX_CAST_HIT>& OutHits)
//{
//	if (nullptr == m_pBody)
//		return false;
//
//	OutHits.clear();
//
//	RefConst<Shape> pShape = m_pBody->GetShape();
//	RMat44 StartTransform = m_pBody->GetWorldTransform();
//
//	Vec3 Direction = LoadVec3(vDir).Normalized() * fDistance;
//	RShapeCast ShapeCastIn = RShapeCast::sFromWorldTransform(
//		pShape, Vec3::sReplicate(1.f), StartTransform, Direction);
//
//	ShapeCastSettings CastSettings;
//}

const JPH::Array<Vec3> CRigidbody::ConvertToArrayVec3(CModel* pModel)
{
	JPH::Array<Vec3> Vertices;

	vector<_float3> ModelVertices = pModel->Get_VerticesPos(0);

	for (size_t i = 0; i < ModelVertices.size(); ++i)
		Vertices.push_back(LoadVec3(ModelVertices[i]));

	return Vertices;
}

const JPH::Array<Float3> CRigidbody::ConvertToArrayFloat3(CModel_Streaming* pModel, const _float3& vScale, _uint iIndex)
{
	JPH::Array<Float3> Vertices;

	vector<_float3> ModelVertices = pModel->Get_VerticesPos(iIndex);

	for (size_t i = 0; i < ModelVertices.size(); ++i)
	{
		_vector Vertex = XMVector3TransformCoord(XMLoadFloat3(&ModelVertices[i]), XMMatrixScaling(vScale.x, vScale.y, vScale.z));
		Vertices.push_back(Float3(Vertex.m128_f32[0], Vertex.m128_f32[1], Vertex.m128_f32[2]));
		//Vertices.push_back(Float3(ModelVertices[i].x, ModelVertices[i].y, ModelVertices[i].z));
	}

	return Vertices;
}

const JPH::Array<IndexedTriangle> CRigidbody::ConvertToArrayTri(CModel_Streaming* pModel, _uint iIndex)
{
	JPH::Array<IndexedTriangle> Indices;

	vector<_uint> ModelIndices = pModel->Get_Indices(iIndex);

	for (size_t i = 0; i < ModelIndices.size(); i += 3)
		Indices.push_back(IndexedTriangle(ModelIndices[i], ModelIndices[i + 1], ModelIndices[i + 2]));

	return Indices;
}

void CRigidbody::Make_MeshShape(void* pArg)
{
	MESHBODY_DESC* pDesc = static_cast<MESHBODY_DESC*>(pArg);

	
	//m_iNumMesh = pDesc->pModel->Get_NumMesh();
	m_iNumMesh = pDesc->pModel->Get_NumMesh(0);
	
	m_ppMeshBodies = new Body*[m_iNumMesh];
	m_pMeshBodyIDs = new BodyID[m_iNumMesh];

	for (_uint i = 0; i < m_iNumMesh; ++i)
	{
		RefConst<Shape> BodyShape;

		Ref<MeshShapeSettings> MeshSetting;
		MeshSetting = new MeshShapeSettings(ConvertToArrayFloat3(pDesc->pModel, pDesc->vScale, i), ConvertToArrayTri(pDesc->pModel, i));
		BodyShape = MeshSetting->Create().Get();

		BodyCreationSettings bodySetting(
			BodyShape,																					// Shape
			Vec3(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z),								// Position
			Quat(pDesc->vQuat.x, pDesc->vQuat.y, pDesc->vQuat.z, pDesc->vQuat.w),	// Quat
			pDesc->eType,																				// Motion Type
			ObjectLayer(pDesc->iLayer)																// Collision Layer
		);

		// SetUp UserData (CollisionData)
		m_tCollisionData.pComponent = this;
		bodySetting.mUserData = reinterpret_cast<uint64>(&m_tCollisionData);

		// Sensor에 감지될 타입은 true로 설정한다.

		m_ppMeshBodies[i] = m_pGameInstance->Register_Body(bodySetting, &m_pBodyInterface);
		m_pMeshBodyIDs[i] = m_ppMeshBodies[i]->GetID();
	}
}

void CRigidbody::Ready_Body(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape)
{
	BodyCreationSettings bodySetting(
		BodyShape,									// Shape
		Vec3(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z),				// Position
		Quat(pDesc->vQuat.x, pDesc->vQuat.y, pDesc->vQuat.z, pDesc->vQuat.w),	// Quat
		pDesc->eType,								// Motion Type
		ObjectLayer(pDesc->iLayer)				// Collision Layer
	);
	MassProperties mp;
	mp.ScaleToMass(1.f);

	bodySetting.mMassPropertiesOverride = mp;
	// Custom Mass SetUp (Default Mass X)
	bodySetting.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	
	if(EMotionType::Kinematic == pDesc->eType)
		bodySetting.mIsSensor = true;
	
	// SetUp UserData (CollisionData)
	m_tCollisionData.pComponent = this;
	bodySetting.mUserData = reinterpret_cast<uint64>(&m_tCollisionData);

	m_pBody = m_pGameInstance->Register_Body(bodySetting, &m_pBodyInterface);
	m_BodyID = m_pBody->GetID();
}

void CRigidbody::Ready_Character(RIGIDBODY_DESC* pDesc, RefConst<Shape> BodyShape)
{
	CharacterSettings CharacterSetting;
	CharacterSetting.mLayer = ObjectLayer(pDesc->iLayer);
	CharacterSetting.mFriction = 1.f;
	CharacterSetting.mGravityFactor = 1.f;
	CharacterSetting.mShape = BodyShape;

	m_pCharacter = m_pGameInstance->Register_Character(CharacterSetting, LoadVec3(pDesc->vPos), LoadQuat(pDesc->vQuat), &m_tCollisionData);
	ASSERT_CRASH(m_pCharacter);
	m_pCharacter->AddToPhysicsSystem();

	m_BodyID = m_pCharacter->GetBodyID();
}

CRigidbody* CRigidbody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRigidbody* pInstance = new CRigidbody(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Rigidbody");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CRigidbody::Clone(void* pArg)
{
	CRigidbody* pClone = new CRigidbody(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Rigidbody (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CRigidbody::Free()
{
	__super::Free();

	m_tCollisionData.pComponent = nullptr;
	m_tCollisionData.pDesc = nullptr;

	// Body Clear
	if (true == m_isClone)
	{
		if (SHAPE::MESH == m_eShape)
		{
			if(true == m_isActive)
				for (_uint i = 0; i < m_iNumMesh; ++i)
					m_pBodyInterface->RemoveBody(m_pMeshBodyIDs[i]);

			Safe_Delete_Array(m_pMeshBodyIDs);
			Safe_Delete_Array(m_ppMeshBodies);
		}
		else if(true == m_isActive)
			m_pBodyInterface->RemoveBody(m_BodyID);
	}

	if (nullptr != m_pCharacter)
		m_pCharacter->RemoveFromPhysicsSystem();
	Safe_Delete(m_pCharacter);
}

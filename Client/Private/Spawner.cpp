#include "ClientPch.h"
#include "Spawner.h"
#include "GameSystem.h"


CSpawner::CSpawner(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSpawner::CSpawner(const CSpawner& Prototype)
	: CGameObject{ Prototype }
	, m_pGameSystem {CGameSystem::GetInstance()}
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CSpawner::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSpawner::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	//m_pGameSystem = CGameSystem::GetInstance();
	//Safe_AddRef(m_pGameSystem);
	SPAWNERDESC* pDesc = static_cast<SPAWNERDESC*>(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat4(&pDesc->vPosition), 1.f));

	Ready_Component(pDesc);
	m_strMonsterKey = pDesc->strMonsterKey;
	m_fSpawnTime = pDesc->fSpawnTime;
	m_iNumSpawnObjects = m_strMonsterKey.size();
#pragma region Information_Seperate
	for (_uint i = 0; i < m_iNumSpawnObjects; ++i)
	{
		//_vector vTransition = XMVectorSetW(XMLoadFloat3(&pDesc->vSpawnPositions[i]), 1.f);
		_float4x4 InitMatrix{};
		XMStoreFloat4x4(&InitMatrix, XMMatrixTranslation(pDesc->vSpawnPositions[i].x, pDesc->vSpawnPositions[i].y, pDesc->vSpawnPositions[i].z));
		m_SpawnMatrix.push_back(InitMatrix);
	}
#pragma endregion

#pragma region Information_OnePoint
	//_float3 vPositionOffsets[3] = {_float3(0.f,0.f,0.f),_float3(1.f,0.f,-1.f), _float3(-1.f,0.f,-1.f) };
	//for (_uint i = 0; i < m_iNumSpawnObjects; ++i)
	//{
	//	_vector vTransition = XMVectorSetW(XMLoadFloat3(&pDesc->vSpawnPosition), 1.f) + XMLoadFloat3(&vPositionOffsets[i]);
	//	_matrix WorldMatrix = XMMatrixAffineTransformation(XMVectorSet(1.f, 1.f, 1.f, 0.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), vTransition);
	//	WorldMatrix = WorldMatrix * XMMatrixRotationQuaternion(vQuaternion) * XMMatrixTranslation(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z);
	//	
	//	_float4x4 InitMatrix{};
	//	XMStoreFloat4x4(&InitMatrix, WorldMatrix);
	//
	//	m_SpawnMatrix.push_back(InitMatrix);
	//}
#pragma endregion
	m_isClone = true;
    return S_OK;
}

void CSpawner::Priority_Update(_float fTimeDelta)
{
	if (m_fTimeAcc <= m_fSpawnTime)
		m_fTimeAcc += fTimeDelta;
	m_isMonsterExist = false;
}

void CSpawner::Update(_float fTimeDelta)
{
	
}

void CSpawner::Late_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::RD_DEBUG, this)))
		return;
#endif
}

void CSpawner::Render()
{
#ifdef _DEBUG
	m_pRigidBodyCom->Render();
#endif // _DEBUG
}

void CSpawner::Ready_Component(SPAWNERDESC* pDesc)
{
	CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.eBodyType = CRigidbody::BODY;
	RigidbodyDesc.eShape = SHAPE::BOX;
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vExtent;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &RigidbodyDesc)))
		CRASH("Rigidbody");

	m_pRigidBodyCom->SetUp_CallBack(COLLIDE_STATE::DURING, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		OnCollide_During(iLayer, pDesc, Manifold);
		});
}

void CSpawner::OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{

}

CSpawner* CSpawner::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpawner* pInstance = new CSpawner(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CSpawner");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSpawner::Clone(void* pArg)
{
	CSpawner* pClone = new CSpawner(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CSpawner (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSpawner::Free()
{
	__super::Free();

	if(m_isClone)
		Safe_Release(m_pGameSystem);
	Safe_Release(m_pRigidBodyCom);
}

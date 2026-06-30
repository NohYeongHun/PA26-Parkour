#include"EnginePch.h"
#include"Model_Instance.h"
#include"Material.h"
#include"Mesh_Instance.h"

CModel_Instance::CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CComponent(pDevice,pContext)
{
}

CModel_Instance::CModel_Instance(const CModel_Instance& Prototype)
    :CComponent(Prototype),
	m_eType{ Prototype.m_eType },
	m_iNumMeshes{ Prototype.m_iNumMeshes },
	m_iNumMaterials{ Prototype.m_iNumMaterials },
	m_Materials{ Prototype.m_Materials },
	m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
{
	//메쉬는 깊은복사하되, m_pVB, m_pIB와같이 메쉬의 정보는 얕은복사.
	for (auto& pMesh : Prototype.m_Meshes)
		m_Meshes.push_back(pMesh->Clone(nullptr));

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);
}

void CModel_Instance::Sync_RootNode(CTransform* pOwnerTransform, CNavigation* pOwnerNavigation, _float fTimeDelta)
{
}

void CModel_Instance::Register_Notify(const _string& strFilePath, const vector<function<void()>>& Functions)
{
}

HRESULT CModel_Instance::Initialize_Prototype(MODELTYPE eType, _fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit, void* pArg)
{
    m_eType = eType;
	
	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	ifstream InputFile(pFilePath, ios::binary);
	if (false == InputFile.is_open())
	{
		MSG_BOX("Failed Open : Model");
		return E_FAIL;
	}

	if (FAILED(Ready_Mesh(InputFile, IsEdit, pArg)))
		return E_FAIL;

	if (FAILED(Ready_Material(pFilePath)))
		return E_FAIL;

	InputFile.close();

    return S_OK;
}

HRESULT CModel_Instance::Initialize_Clone(void* pArg)
{
	//?꾩떆
	for (auto& pMesh : m_Meshes)
		pMesh->Initialize_Clone(pArg);

    return S_OK;
}

HRESULT CModel_Instance::Render(_uint iMeshIndex)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;
	
	m_Meshes[iMeshIndex]->Render();
	return S_OK;
}

HRESULT CModel_Instance::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources(pDC)))
		return E_FAIL;
	m_Meshes[iMeshIndex]->Render(pDC);

	return S_OK;
}

#ifdef _DEBUG
_bool CModel_Instance::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
	_float fMin = FLT_MAX;
	for (_uint i = 0; i < m_iNumMeshes; ++i)
	{
		_float fDistance = {};
		if (true == m_Meshes[i]->Is_Picked(vRayPos, vRayDir, &fDistance) && fMin > fDistance)
			fMin = fDistance;
	}

	if (fMin < FLT_MAX)
	{
		*pDistance = fMin;
		return true;
	}

	return false;
}
void CModel_Instance::Change_InstanceInfo(_uint iNumInstance, _fmatrix fMatrix)
{
	for (auto& pMesh : m_Meshes)
		pMesh->Change_InstanceInfo(iNumInstance, fMatrix);
}
#endif

HRESULT CModel_Instance::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel_Instance::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType);
}

HRESULT CModel_Instance::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex, pEffect);
}

HRESULT CModel_Instance::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, pEffect);
}
HRESULT CModel_Instance::Ready_Mesh(ifstream& InputFile, _bool IsEdit, void* pArg)
{
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh_Instance* pMesh = CMesh_Instance::Create(m_pDevice, m_pContext, XMLoadFloat4x4(&m_PreTransformMatrix), IsEdit, pArg, InputFile, m_MinPos, m_MaxPos);

		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel_Instance::Ready_Material(const _char* pFilePath)
{
	_char szMaterialFilePath[MAX_PATH] = {};
	_char szMaterialDrivePath[MAX_PATH] = {};
	_char szMaterialDirPath[MAX_PATH] = {};
	_char szMaterialFileName[MAX_PATH] = {};
	_splitpath_s(pFilePath, szMaterialDrivePath, MAX_PATH, szMaterialDirPath, MAX_PATH, szMaterialFileName, MAX_PATH, nullptr, 0);

	strcpy_s(szMaterialFilePath, szMaterialDrivePath);
	strcat_s(szMaterialFilePath, szMaterialDirPath);
	strcat_s(szMaterialFilePath, "Mat/");
	strcat_s(szMaterialFilePath, szMaterialFileName);
	strcat_s(szMaterialFilePath, ".json");

	ifstream MaterialFile(szMaterialFilePath);
	if (false == MaterialFile.is_open())
	{
		MSG_BOX("Failed Open : Material");
		return E_FAIL;
	}

	json MaterialsData;
	MaterialFile >> MaterialsData;

	m_iNumMaterials = MaterialsData["NumMaterial"];

	for (auto& MaterialData : MaterialsData["Materials"])
	{
		CMaterial* pMeshMaterial = CMaterial::Create(m_pDevice, m_pContext, MaterialData);
		if (nullptr == pMeshMaterial)
			return E_FAIL;
		m_Materials.push_back(pMeshMaterial);
	}

	MaterialFile.close();

	return S_OK;
}

CModel_Instance* CModel_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit, void* pArg)
{
	CModel_Instance* pInstance = new CModel_Instance(pDevice, pContext);

	MODELTYPE eType = MODELTYPE::MAP;

	if (FAILED(pInstance->Initialize_Prototype(eType, PreTransformMatrix, pFilePath, IsEdit, pArg)))
	{
		MSG_BOX("Failed to Create : Model_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel_Instance::Clone(void* pArg)
{
	CModel_Instance* pClone = new CModel_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model_Instance (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CModel_Instance::Free()
{
	__super::Free();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);

	m_Materials.clear();
}

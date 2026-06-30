#include"EnginePch.h"
#include "Model_Instance_FireFly.h"
#include"Mesh_Instance_FireFly.h"
#include"Material.h"

CModel_Instance_FireFly::CModel_Instance_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent(pDevice,pContext)
{
}

CModel_Instance_FireFly::CModel_Instance_FireFly(const CModel_Instance_FireFly& Prototype)
	:CComponent(Prototype),
	m_iNumMeshes{ Prototype.m_iNumMeshes },
	m_iNumMaterials{ Prototype.m_iNumMaterials },
	m_Materials{ Prototype.m_Materials },
	m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
{
	for (auto& pMesh : Prototype.m_Meshes)
		m_Meshes.push_back(pMesh->Clone(nullptr));

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);
}

HRESULT CModel_Instance_FireFly::Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit, void* pArg)
{
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

HRESULT CModel_Instance_FireFly::Initialize_Clone(void* pArg)
{
	for (auto& pMesh : m_Meshes)
		pMesh->Initialize_Clone(pArg);

	//
	return S_OK;
}

HRESULT CModel_Instance_FireFly::Render(_uint iMeshIndex)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources()))
		return E_FAIL;

	m_Meshes[iMeshIndex]->Render();
	return S_OK;
}

HRESULT CModel_Instance_FireFly::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (FAILED(m_Meshes[iMeshIndex]->Bind_Resources(pDC)))
		return E_FAIL;
	m_Meshes[iMeshIndex]->Render(pDC);

	return S_OK;
}
HRESULT CModel_Instance_FireFly::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex);
}

HRESULT CModel_Instance_FireFly::Bind_Materials(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType);
}

HRESULT CModel_Instance_FireFly::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return E_FAIL;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, iTextureIndex, pEffect);
}

HRESULT CModel_Instance_FireFly::Bind_Materials(CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (iMeshIndex >= m_Meshes.size())
		return S_OK;

	return m_Materials[m_Meshes[iMeshIndex]->Get_MaterialIndex()]->Bind_Resource(pShader, pConstantName, eTextureType, pEffect);
}
#ifdef _DEBUG
void CModel_Instance_FireFly::Change_Pos(_fvector vPos)
{
	for (size_t i = 0; i < m_iNumMeshes; ++i)
		m_Meshes[i]->FireFly_Move(vPos);
}
#endif
HRESULT CModel_Instance_FireFly::Ready_Mesh(ifstream& InputFile, _bool IsEdit, void* pArg)
{
	InputFile.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));

	for (size_t i = 0; i < m_iNumMeshes; ++i)
	{
		CMesh_Instance_FireFly* pMesh = CMesh_Instance_FireFly::Create(m_pDevice, m_pContext, XMLoadFloat4x4(&m_PreTransformMatrix), IsEdit, pArg, InputFile, m_MinPos, m_MaxPos);

		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel_Instance_FireFly::Ready_Material(const _char* pFilePath)
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

CModel_Instance_FireFly* CModel_Instance_FireFly::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit, void* pArg)
{
	CModel_Instance_FireFly* pInstance = new CModel_Instance_FireFly(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, pFilePath, IsEdit, pArg)))
	{
		MSG_BOX("Failed to Create : Model_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel_Instance_FireFly::Clone(void* pArg)
{
	CModel_Instance_FireFly* pClone = new CModel_Instance_FireFly(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model_Instance (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CModel_Instance_FireFly::Free()
{
	__super::Free();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);

	m_Materials.clear();
}

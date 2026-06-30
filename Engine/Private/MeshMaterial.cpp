#include "EnginePch.h"
#include "MeshMaterial.h"

#include "Shader.h"
#include "DeferredShader.h"

CMeshMaterial::CMeshMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CMeshMaterial::Initialize(const _char* pFilePath, const json& MaterialData)
{
	_char szDrivePath[MAX_PATH] = {};
	_char szDirPath[MAX_PATH] = {};
	_char szBasePath[MAX_PATH] = {};
	_splitpath_s(pFilePath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, nullptr, 0, nullptr, 0);
	strcpy_s(szBasePath, szDrivePath);
	strcat_s(szBasePath, szDirPath);
	strcat_s(szBasePath, "Tex/");

	if (true == MaterialData.contains("Diffuse"))
	{
		if (FAILED(Load_File(szBasePath, MaterialData, "Diffuse", TEXTURETYPE::DIFFUSE)))
			return E_FAIL;
	}

	if (true == MaterialData.contains("Normal"))
	{
		if (FAILED(Load_File(szBasePath, MaterialData, "Normal", TEXTURETYPE::NORMAL)))
			return E_FAIL;
	}

	if (true == MaterialData.contains("Mask"))
	{
		if (FAILED(Load_File(szBasePath, MaterialData, "Mask", TEXTURETYPE::MASK)))
			return E_FAIL;
	}

	if (true == MaterialData.contains("Emissive"))
	{
		if (FAILED(Load_File(szBasePath, MaterialData, "Emissive", TEXTURETYPE::EMISSIVE)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMeshMaterial::Bind_Resource(CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iTextureIndex >= m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Texture(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)][iTextureIndex]);
}

HRESULT CMeshMaterial::Bind_Resource(CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Textures(pConstantName, &m_SRVs[ENUM_CLASS(eTextureType)].front(), m_SRVs[ENUM_CLASS(eTextureType)].size());
}

HRESULT CMeshMaterial::Bind_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iTextureIndex >= m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Texture(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)][iTextureIndex], pEffect);
}

HRESULT CMeshMaterial::Bind_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Textures(pConstantName, &m_SRVs[ENUM_CLASS(eTextureType)].front(), m_SRVs[ENUM_CLASS(eTextureType)].size(), pEffect);
}

HRESULT CMeshMaterial::Clear_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Clear_Textures(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)].size(), pEffect);
}

HRESULT CMeshMaterial::Load_File(const _char* pFilePath, const json& MaterialData, const _char* pTextureTag, TEXTURETYPE eTextureType)
{
	json TextureData = MaterialData[pTextureTag];
	_uint iTextureCnt = TextureData["TextureCnt"];
	if (0 == iTextureCnt)
		return S_OK;

	for (size_t i = 0; i < iTextureCnt; ++i)
	{
		_char szFilePath[MAX_PATH] = {};
		strcpy_s(szFilePath, pFilePath);
		string strFileName = TextureData["FileName"][i];
		strcat_s(szFilePath, strFileName.c_str());

		_tchar szExtPath[MAX_PATH] = {};
		_tchar pPath[MAX_PATH] = {};
		MultiByteToWideChar(CP_ACP, 0, szFilePath, -1, pPath, MAX_PATH);
		_tchar szDrivePath[MAX_PATH] = {};
		_tchar szDirPath[MAX_PATH] = {};
		_tchar szNamePath[MAX_PATH] = {};
		_wsplitpath_s(pPath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, szNamePath, MAX_PATH, szExtPath, MAX_PATH);

		HRESULT hr;
		ID3D11ShaderResourceView* pSRV = { nullptr };

		_tchar szDDSPath[MAX_PATH] = {};

		lstrcpy(szDDSPath, szDrivePath);
		lstrcat(szDDSPath, szDirPath);
		lstrcat(szDDSPath, szNamePath);
		lstrcat(szDDSPath, TEXT(".dds"));

		// dds
		hr = CreateDDSTextureFromFile(m_pDevice, szDDSPath, nullptr, &pSRV);

		if (FAILED(hr))
		{
			// png
			hr = CreateWICTextureFromFile(m_pDevice, pPath, nullptr, &pSRV);
			if (FAILED(hr))
			{
				MessageBoxW(NULL, pPath, L"Texture", MB_OK);
				return E_FAIL;
			}
		}

		m_SRVs[ENUM_CLASS(eTextureType)].push_back(pSRV);
	}

	return S_OK;
}

CMeshMaterial* CMeshMaterial::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, const json& MaterialData)
{
	CMeshMaterial* pInstance = new CMeshMaterial(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pFilePath, MaterialData)))
	{
		MSG_BOX("Failed to Create : MeshMaterial");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshMaterial::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	for (size_t i = 0; i < ENUM_CLASS(TEXTURETYPE::END); ++i)
	{
		for (auto& pSRV : m_SRVs[i])
			Safe_Release(pSRV);
		m_SRVs[i].clear();
	}
}

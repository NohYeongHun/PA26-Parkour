#include "EnginePch.h"
#include "Material.h"

#include "GameInstance.h"

CMaterial::CMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CMaterial::Initialize(const json& MaterialData)
{
	if (true == MaterialData.contains("Diffuse"))
		Store_Texture(MaterialData, "Diffuse", TEXTURETYPE::DIFFUSE);
	
	if (true == MaterialData.contains("Normal"))
		Store_Texture(MaterialData, "Normal", TEXTURETYPE::NORMAL);
	
	if (true == MaterialData.contains("Mask"))
		Store_Texture(MaterialData, "Mask", TEXTURETYPE::MASK);
	
	if (true == MaterialData.contains("Emissive"))
		Store_Texture(MaterialData, "Emissive", TEXTURETYPE::EMISSIVE);
	
	return S_OK;
}

HRESULT CMaterial::Bind_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect)
{
	if (iTextureIndex >= m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Texture(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)][iTextureIndex], pEffect);
}

HRESULT CMaterial::Bind_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Bind_Textures(pConstantName, &m_SRVs[ENUM_CLASS(eTextureType)].front(), m_SRVs[ENUM_CLASS(eTextureType)].size(), pEffect);
}

HRESULT CMaterial::Bind_Resource(CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex)
{
	if (iTextureIndex >= m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)][iTextureIndex]);
}


HRESULT CMaterial::Bind_Resource(CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;

	return pShader->Bind_Textures(pConstantName, &m_SRVs[ENUM_CLASS(eTextureType)].front(), m_SRVs[ENUM_CLASS(eTextureType)].size());
}

HRESULT CMaterial::Clear_Resource(CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect)
{
	if (0 == m_SRVs[ENUM_CLASS(eTextureType)].size())
		return E_FAIL;
	return pShader->Clear_Textures(pConstantName, m_SRVs[ENUM_CLASS(eTextureType)].size(), pEffect);
}

void CMaterial::Store_Texture(const json& MaterialData, const _char* pTextureTag, TEXTURETYPE eTextureType)
{
	json TextureData = MaterialData[pTextureTag];
	_uint iTextureCnt = TextureData["TextureCnt"];
	if (0 == iTextureCnt)
		return;

	for (size_t i = 0; i < iTextureCnt; ++i)
	{
		string strFileName = TextureData["FileName"][i];

		size_t CutPos = strFileName.find(".");

		// Ext 제외
		if (CutPos != std::string::npos)
			strFileName = strFileName.substr(0, CutPos);
		
		ID3D11ShaderResourceView* pSRV = m_pGameInstance->Get_Resource(strFileName);
		if (pSRV)
			m_SRVs[ENUM_CLASS(eTextureType)].push_back(pSRV);
	}
}

CMaterial* CMaterial::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const json& MaterialData)
{
	CMaterial* pInstance = new CMaterial(pDevice, pContext);

	if (FAILED(pInstance->Initialize(MaterialData)))
	{
		int a = 0;
		CRASH("Material");
	}

	return pInstance;
}

void CMaterial::Free()
{
	__super::Free();
	for (auto& Pair : m_SRVs)
	{
		for (auto& pSRV : Pair)
			Safe_Release(pSRV);
		Pair.clear();
	}
	m_SRVs->clear();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

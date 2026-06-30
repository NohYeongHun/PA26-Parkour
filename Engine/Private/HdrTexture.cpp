#include "EnginePch.h"
#include "HdrTexture.h"
#include "Shader.h"

CHdrTexture::CHdrTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

HRESULT CHdrTexture::Initialize_Prototype(const _tchar* pFilePath, _uint iNumTextures)
{
	m_iNumTextures = iNumTextures;

	_tchar      szExt[MAX_PATH] = {};

	// Path Split => ?뺤옣?먮쭔 異붿텧
	_wsplitpath_s(pFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

	for (_uint i = 0; i < iNumTextures; ++i)
	{
		_tchar szFileName[MAX_PATH] = {};
		wsprintf(szFileName, pFilePath, i);

		TexMetadata Data = {};

		ScratchImage OutImage = {};

		if (FAILED(LoadFromHDRFile(szFileName, &Data, OutImage)))
			CRASH("Failed to Load HDR File");

		m_MaxFrames.push_back(static_cast<_float>(Data.height));
		
		ID3D11Texture2D* pTexture = {};
		D3D11_TEXTURE2D_DESC TextureDesc = {};

		TextureDesc.Width = Data.width;
		TextureDesc.Height = Data.height;
		TextureDesc.MipLevels = Data.mipLevels;
		TextureDesc.ArraySize = Data.arraySize;

		TextureDesc.Format = Data.format;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.SampleDesc.Count = 1;

		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.CPUAccessFlags = 0;
		TextureDesc.MiscFlags = Data.miscFlags;

		CreateTexture(m_pDevice, OutImage.GetImages(), OutImage.GetImageCount(), Data, reinterpret_cast<ID3D11Resource**>(&pTexture));
		ASSERT_CRASH(pTexture);

		D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};
		SRV_Desc.Format = Data.format;
		SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRV_Desc.Texture2D.MostDetailedMip = 0;
		SRV_Desc.Texture2D.MipLevels = Data.mipLevels;

		ID3D11ShaderResourceView* pSRV = { nullptr };

		if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRV_Desc, &pSRV)))
			ASSERT_CRASH(pSRV);

		m_HdrSRVs.push_back(pSRV);

		Safe_Release(pTexture);
	}
    return S_OK;
}

HRESULT CHdrTexture::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHdrTexture::Bind_Shader_Resource(CShader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
	if (m_iNumTextures < iTextureIndex)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_HdrSRVs[iTextureIndex]);
}

HRESULT CHdrTexture::Bind_Shader_Resource(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Textures(pConstantName, m_HdrSRVs.data(), m_iNumTextures);
}

ID3D11ShaderResourceView* CHdrTexture::Get_SRV(_uint iTextureIndex)
{
	if (m_iNumTextures < iTextureIndex)
		return nullptr;

	return m_HdrSRVs[iTextureIndex];
}

_float CHdrTexture::Get_MaxFrame(_uint iTextureIndex)
{
	if (m_iNumTextures < iTextureIndex)
		return 0.f;

	return m_MaxFrames[iTextureIndex];
}

CHdrTexture* CHdrTexture::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _uint iNumTextures)
{
	CHdrTexture* pInstance = new CHdrTexture(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype(pFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : CHdrTexture");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CHdrTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_HdrSRVs)
		Safe_Release(pSRV);
	m_HdrSRVs.clear();
}

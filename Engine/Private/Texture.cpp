#include "EnginePch.h"
#include "Texture.h"

#include "Shader.h"

CTexture::CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CTexture::CTexture(const CTexture& Prototype)
    : CComponent { Prototype },
    m_iNumTextures { Prototype.m_iNumTextures },
    m_SRVs { Prototype.m_SRVs }
{
    for (auto& pSRV : m_SRVs)
        Safe_AddRef(pSRV);
}

HRESULT CTexture::Initialize_Prototype(const _tchar* pFilePath, _uint iNumTexture)
{
    m_iNumTextures = iNumTexture;

    _tchar      szExt[MAX_PATH] = {};

    // Path Split => ?뺤옣?먮쭔 異붿텧
    _wsplitpath_s(pFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

    for (size_t i = 0; i < m_iNumTextures; ++i)
    {
        _tchar szFileName[MAX_PATH] = {};
        wsprintf(szFileName, pFilePath, i);

        ID3D11ShaderResourceView* pSRV = { nullptr };

        HRESULT hr = {};

        if (0 == lstrcmp(szExt, TEXT(".dds")))
        {
            hr = CreateDDSTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);
        }
        else if (0 == lstrcmp(szExt, TEXT(".tga")))
        {
            MSG_BOX("TGA");
            return E_FAIL;
        }
        else // dds??Window媛 吏?먰븯???뚯씪
        {
            hr = CreateWICTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);
        }

		if (FAILED(hr))
		{
			MessageBoxW(NULL, szFileName, L"Texture", MB_OK);
            return E_FAIL;
		}
        m_SRVs.push_back(pSRV);
    }

    return S_OK;
}

HRESULT CTexture::Initialize_Clone(void* pArg)
{
    return S_OK;
}

HRESULT CTexture::Bind_Shader_Resource(CShader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
    if (nullptr == pShader || m_iNumTextures <= iTextureIndex)
        return E_FAIL;

    return pShader->Bind_Texture(pConstantName, m_SRVs[iTextureIndex]);
}

HRESULT CTexture::Bind_Shader_Resource(CShader* pShader, const _char* pConstantName)
{
	if (nullptr == pShader)
		return E_FAIL;

	return pShader->Bind_Textures(pConstantName, &m_SRVs.front(), m_iNumTextures);
}

ID3D11ShaderResourceView* CTexture::Get_SRV(_uint iIndex)
{
    if (iIndex >= m_SRVs.size())
    return nullptr;

    return m_SRVs[iIndex];
}

CTexture* CTexture::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _uint iNumTexture)
{
    CTexture* pInstance = new CTexture(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(pFilePath, iNumTexture)))
    {
        MSG_BOX("Failed to Create : Texture");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CTexture::Clone(void* pArg)
{
    CTexture* pClone = new CTexture(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : Texture (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CTexture::Free()
{
    __super::Free();

    for (auto& pSRV : m_SRVs)
        Safe_Release(pSRV);
}

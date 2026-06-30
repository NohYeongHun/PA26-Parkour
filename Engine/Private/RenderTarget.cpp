#include "EnginePch.h"
#include "RenderTarget.h"

#include "Shader.h"
#include "VIBuffer_Rect.h"


CRenderTarget::CRenderTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CRenderTarget::Initialize(_uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor)
{
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = eFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateRenderTargetView(m_pTexture2D, nullptr, &m_pRTV)))
		return E_FAIL;
	 
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pTexture2D, nullptr, &m_pSRV)))
		return E_FAIL;


	m_vClearColor = vClearColor;

    return S_OK;
}

HRESULT CRenderTarget::Bind_Shader_Resource(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Texture(pConstantName, m_pSRV);
}

void CRenderTarget::Clear()
{
	m_pContext->ClearRenderTargetView(m_pRTV, reinterpret_cast<_float*>(&m_vClearColor));
}

HRESULT CRenderTarget::Render(const _wstring& strRT_Name)
{
	ImGui::Begin(WStringToString(strRT_Name).c_str());

	ImGui::Image(reinterpret_cast<ImTextureID>(m_pSRV), ImVec2(500.f, 500.f));

	ImGui::End();

	return S_OK;
}
#ifdef _DEBUG
HRESULT CRenderTarget::Ready_Debug(_float fX, _float fY, _float fSizeX, _float fSizeY)
{
	_uint iNumViewPort = { 1 };
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewPort, &ViewPort);

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(fSizeX, fSizeY, 1.f));
	m_WorldMatrix._41 = fX - ViewPort.Width * 0.5f;
	m_WorldMatrix._42 = -fY + ViewPort.Height * 0.5f;

	return S_OK;
}

HRESULT CRenderTarget::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(pShader->Bind_Texture("g_Texture", m_pSRV)))
		return E_FAIL;

	pShader->Begin(0);
	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

#endif

CRenderTarget* CRenderTarget::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor)
{
	CRenderTarget* pInstance = new CRenderTarget(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWidth, iHeight, eFormat, vClearColor)))
	{
		MSG_BOX("Failed to Create : RenderTarget");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CRenderTarget::Free()
{
	__super::Free();

	Safe_Release(m_pTexture2D);
	Safe_Release(m_pRTV);
	Safe_Release(m_pSRV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

}

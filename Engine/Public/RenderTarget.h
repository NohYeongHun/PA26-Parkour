#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CRenderTarget final : public CBase
{
private:
	explicit CRenderTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderTarget() = default;

public:
	ID3D11RenderTargetView*             Get_RTV() { return m_pRTV; }
	ID3D11Texture2D*					Get_Resource() { return m_pTexture2D; }
	ID3D11ShaderResourceView*           Get_SRV() { return m_pSRV; }

#ifdef _DEBUG
public:
	void								Change_BackBufferColor(const _float4& vClearColor) { m_vClearColor = vClearColor; }
#endif

public:
	HRESULT								Initialize(_uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor);
	HRESULT								Bind_Shader_Resource(class CShader* pShader, const _char* pConstantName);
	void							    Clear();
	HRESULT                             Render(const _wstring& strRT_Name);
#ifdef _DEBUG
	HRESULT								Ready_Debug(_float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT								Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

#endif

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	ID3D11Texture2D*					m_pTexture2D = { nullptr };
	ID3D11RenderTargetView*		        m_pRTV = { nullptr };
	ID3D11ShaderResourceView*		    m_pSRV = { nullptr };

	_float4								m_vClearColor = {};

#ifdef _DEBUG
	_float4x4							m_WorldMatrix = {};
#endif

public:
	static		CRenderTarget*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor);
	virtual		void					Free() override;
};

NS_END
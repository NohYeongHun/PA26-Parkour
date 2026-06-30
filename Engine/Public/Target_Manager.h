#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTarget_Manager : public CBase
{
private:
	explicit CTarget_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTarget_Manager() = default;

public:
	ID3D11Texture2D* Get_RT_Resource(const _wstring& strTargetTag);
	ID3D11ShaderResourceView* Get_RT_SRV(const _wstring& strTargetTag);
#ifdef _DEBUG
	ID3D11ShaderResourceView* Get_Debug_RT_Resource(const _wstring& strTargetTag);
public:
	void					Change_BackBufferColor(const _wstring& strTargetTag, const _float4& vClearColor);

#endif

public:
	HRESULT		Bind_OpenRT(OPEN_RT eRT, class CShader* pShader, const _char* pConstantName);

public:
	HRESULT		Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor);
	HRESULT		Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT		Bind_Shader_Resource(const _wstring& strTargetTag, class CShader* pShader, const _char* pConstantName);
	HRESULT		Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isClear);
	HRESULT		SetUp_MRT(ID3D11DeviceContext* pContext, const _wstring& strMRTTag);
	void	    End_MRT();
	HRESULT		Clear_RT(const _wstring& strTargetTag);
	HRESULT     Render();


	HRESULT		Ready_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	void		AddRemoveRT(const _wstring& strTargetTag, class CRenderTarget* pRT);
#ifdef _DEBUG
	HRESULT		Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

#endif

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	ID3D11RenderTargetView* m_pBackBuffer = { nullptr };
	ID3D11DepthStencilView* m_pOriginalDSV = { nullptr };

	map<const _wstring, class CRenderTarget*> m_RenderTargets;
	map<const _wstring, list<class CRenderTarget*>> m_MRTs;
	map<const _wstring, class CRenderTarget*> m_DebugRenderRT;

private:
	class CRenderTarget*				Find_RenderTarget(const _wstring& strTargetTag);
	list<class CRenderTarget*>*	Find_MRT(const _wstring& strMRTTag);

public:
	static		CTarget_Manager*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void						Free() override;
};

NS_END
#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGraphic_Device final : public CBase
{
private:
	explicit CGraphic_Device();
	virtual ~CGraphic_Device() = default;

public:
	void				SetUp_RTV();

public:
	HRESULT			Initialize(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);

	HRESULT			Clear_BackBuffer_View(const _float4* pClearColor);
	HRESULT			Clear_DepthStencil_View();
	HRESULT			Present();

private:
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*			m_pContext = { nullptr };

	IDXGISwapChain*				m_pSwapChain = { nullptr };

	ID3D11RenderTargetView*	m_pBackBufferRTV = { nullptr };
	ID3D11DepthStencilView*		m_pDepthStencilView = { nullptr };

private:
	HRESULT			Ready_SwapChain(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY);
	HRESULT			Ready_BackBufferRenderTargetView();
	HRESULT			Ready_DepthStencilView(_uint iWinSizeX, _uint iWinSizeY);

public:
	static		CGraphic_Device*	Create(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	virtual		void					Free() override;
};

NS_END
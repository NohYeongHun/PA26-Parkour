#include "EnginePch.h"
#include "Graphic_Device.h"

CGraphic_Device::CGraphic_Device()
{
}

HRESULT CGraphic_Device::Initialize(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    _uint   iFlag = 0;

#ifdef _DEBUG
    iFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
    D3D_FEATURE_LEVEL   FeatureLV = {};

    // 洹몃옒???μ튂 珥덇린??
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, iFlag,
        nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &FeatureLV, &m_pContext)))
        return E_FAIL;

    // SwapChain 媛앹껜 諛?Back Buffer ?앹꽦
    if (FAILED(Ready_SwapChain(hWnd, eMode, iWinSizeX, iWinSizeY)))
        return E_FAIL;
    // Render Target View ?앹꽦
    if (FAILED(Ready_BackBufferRenderTargetView()))
        return E_FAIL;
    // Depth Stencil View ?앹꽦
    if (FAILED(Ready_DepthStencilView(iWinSizeX, iWinSizeY)))
        return E_FAIL;

    // ?μ튂??RTV, DepthStencilView ?뗮똿
    ID3D11RenderTargetView* pRTVs[] = {
        m_pBackBufferRTV,
    };

    m_pContext->OMSetRenderTargets(1, pRTVs, m_pDepthStencilView);

    D3D11_VIEWPORT ViewPortDesc = {};
    ViewPortDesc.TopLeftX = 0;
    ViewPortDesc.TopLeftY = 0;
    ViewPortDesc.Width = static_cast<_float>(iWinSizeX);
    ViewPortDesc.Height = static_cast<_float>(iWinSizeY);
    ViewPortDesc.MinDepth = 0.f;
    ViewPortDesc.MaxDepth = 1.f;

    m_pContext->RSSetViewports(1, &ViewPortDesc);

    *ppDevice = m_pDevice;
    *ppContext = m_pContext;

    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);

    return S_OK;
}

HRESULT CGraphic_Device::Clear_BackBuffer_View(const _float4* pClearColor)
{
    if (nullptr == m_pContext)
        return E_FAIL;

    m_pContext->ClearRenderTargetView(m_pBackBufferRTV, reinterpret_cast<const _float*>(pClearColor));

    return S_OK;
}

HRESULT CGraphic_Device::Clear_DepthStencil_View()
{
    if (nullptr == m_pContext)
        return E_FAIL;

    m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    return S_OK;
}

HRESULT CGraphic_Device::Present()
{
    if (nullptr == m_pSwapChain)
        return E_FAIL;

    return m_pSwapChain->Present(1, 0);
}

HRESULT CGraphic_Device::Ready_SwapChain(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY)
{
    IDXGIDevice* pDevice = { nullptr };
    m_pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDevice));

    IDXGIAdapter* pAdapter = { nullptr };
    pDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pAdapter));

    IDXGIFactory* pFactory = { nullptr };
    pAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory));

    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    // Back Buffer Size
    SwapChainDesc.BufferDesc.Width = iWinSizeX;
    SwapChainDesc.BufferDesc.Height = iWinSizeY;

    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;

    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    // 硫???섑뵆留?
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.SampleDesc.Count = 1;

    SwapChainDesc.OutputWindow = hWnd;
    SwapChainDesc.Windowed = static_cast<BOOL>(eMode);
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (FAILED(pFactory->CreateSwapChain(m_pDevice, &SwapChainDesc, &m_pSwapChain)))
        return E_FAIL;

    Safe_Release(pFactory);
    Safe_Release(pAdapter);
    Safe_Release(pDevice);

    return S_OK;
}

HRESULT CGraphic_Device::Ready_BackBufferRenderTargetView()
{
    if (nullptr == m_pDevice)
        return E_FAIL;

    ID3D11Texture2D* pBackBufferTexture = { nullptr };

    // SwapChain??媛뽮퀬 ?덈뒗 Texture(Back Buffer) 媛?몄삤湲?
    if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), 
        reinterpret_cast<void**>(&pBackBufferTexture))))
        return E_FAIL;

    // RenderTarget ?⑸룄濡????띿뒪泥?媛앹껜瑜??앹꽦 (Back Buffer 湲곕컲)
    if (FAILED(m_pDevice->CreateRenderTargetView(pBackBufferTexture, 
        nullptr, &m_pBackBufferRTV)))
        return E_FAIL;

    Safe_Release(pBackBufferTexture);

    return S_OK;
}

HRESULT CGraphic_Device::Ready_DepthStencilView(_uint iWinSizeX, _uint iWinSizeY)
{
    if (nullptr == m_pDevice)
        return E_FAIL;

    ID3D11Texture2D* pDepthStencilTexture = { nullptr };

    D3D11_TEXTURE2D_DESC TextureDesc = {};

    TextureDesc.Width = iWinSizeX;
    TextureDesc.Height = iWinSizeY;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    TextureDesc.SampleDesc.Quality = 0;
    TextureDesc.SampleDesc.Count = 1;

    TextureDesc.Usage = D3D11_USAGE_DEFAULT; // ?뺤쟻
    TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    TextureDesc.CPUAccessFlags = 0;
    TextureDesc.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
        return E_FAIL;

    if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pDepthStencilView)))
        return E_FAIL;

    Safe_Release(pDepthStencilTexture);

    return S_OK;
}

CGraphic_Device* CGraphic_Device::Create(HWND hWnd, WINMODE eMode, _uint iWinSizeX, _uint iWinSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    CGraphic_Device* pInstance = new CGraphic_Device();

    if (FAILED(pInstance->Initialize(hWnd, eMode, iWinSizeX, iWinSizeY, ppDevice, ppContext)))
    {
        MSG_BOX("Failed to Create : Graphic_Device");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CGraphic_Device::Free()
{
    __super::Free();

    Safe_Release(m_pSwapChain);
    Safe_Release(m_pBackBufferRTV);
    Safe_Release(m_pDepthStencilView);
	m_pContext->ClearState();
    m_pContext->Flush();
    Safe_Release(m_pContext);

#if defined(DEBUG) || defined(_DEBUG)
    ID3D11Debug* pD3DDebug = { nullptr };
    HRESULT hr = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&pD3DDebug));
    if (SUCCEEDED(hr))
    {
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");

        hr = pD3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker END \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
    }
    Safe_Release(pD3DDebug);
#endif

    Safe_Release(m_pDevice);
}
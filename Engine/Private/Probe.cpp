#include "EnginePch.h"
#include "Probe.h"
#include "StaticObject.h"
#include "GameInstance.h"

CProbe::CProbe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CProbe::Initialize(const _float3& vCenter, const _float& fRange)
{
	_float3 vExtents = _float3(fRange, fRange, fRange);

	m_pBounding = new BoundingBox(vCenter, vExtents);
	ASSERT_CRASH(m_pBounding);

	m_vCenter = _float4(vCenter.x, vCenter.y, vCenter.z, 1.f);
	m_fRange = fRange;

	if (FAILED(Ready_Matrices()))
		return E_FAIL;

	if (FAILED(Ready_Resources()))
		return E_FAIL;

    return S_OK;
}

_bool CProbe::IsInProbe(BoundingBox* pObjectBounding)
{
    return 	m_pBounding->Intersects(*pObjectBounding);
}

_bool CProbe::IsInFrustrum()
{
	return m_pGameInstance->IsIn_WorldSpace(m_pBounding);
}

void CProbe::Fill_Data(ENV_MAP* pOut)
{
	pOut->fRange = m_fRange;
	pOut->vPosition = m_vCenter;
}

void CProbe::Render_SkyBoxs(_float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
	for (auto& pSkyBox : m_SkyBoxs)
		pSkyBox->Render_EnvMap(m_vCenter, ViewMatrix, ProjMatrix);
}

void CProbe::Render_StaticObjects(_float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
	for (auto& pStaticObject : m_StaticObjects)
		pStaticObject->Render_EnvMap(m_vCenter, ViewMatrix, ProjMatrix);
}

HRESULT CProbe::Ready_Matrices()
{
	FACE Faces[6] = {	_float3(1.f, 0.f, 0.f)  , _float3(0.f, 1.f, 0.f),
						_float3(-1.f, 0.f, 0.f) , _float3(0.f, 1.f, 0.f),
						_float3(0.f, 1.f, 0.f)  , _float3(0.f, 0.f, -1.f),
						_float3(0.f, -1.f, 0.f) , _float3(0.f, 0.f, 1.f),
						_float3(0.f, 0.f, 1.f)  , _float3(0.f, 1.f, 0.f),
						_float3(0.f, 0.f, -1.f) , _float3(0.f, 1.f, 0.f) };

	for (_uint i = 0; i < 6; ++i)
	{
		_vector vEye = XMLoadFloat4(&m_vCenter);
		_vector vAt = XMLoadFloat4(&m_vCenter) + XMVectorScale(XMLoadFloat3(&(Faces[i].vDir)), m_fRange);

		_float4x4 ViewMatrix = {};

		XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(vEye, vAt, XMLoadFloat3(&(Faces[i].vUp))));
		
		m_Matrices[ENUM_CLASS(D3DTS::VIEW)].push_back(ViewMatrix);

		_float4x4 ViewMatrixInv = {};
		XMStoreFloat4x4(&ViewMatrixInv, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)][i])));
		m_MatricesInv[ENUM_CLASS(D3DTS::VIEW)].push_back(ViewMatrixInv);

		_float4x4 ProjMatrix = {};
		
		XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(90.f), 1.f, 0.1f, (m_fRange * 2.f)));

		m_Matrices[ENUM_CLASS(D3DTS::PROJ)].push_back(ProjMatrix);

		_float4x4 ProjMatrixInv = {};
		XMStoreFloat4x4(&ProjMatrixInv, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)][i])));
		m_MatricesInv[ENUM_CLASS(D3DTS::PROJ)].push_back(ProjMatrixInv);
	}

	return S_OK;
}

HRESULT CProbe::Ready_Resources()
{
	//RTV + SRV
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	
	TextureDesc.Width = g_iEnvMapSize;
	TextureDesc.Height = g_iEnvMapSize;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 6;

	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;			// DSV, SRV
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture);
	ASSERT_CRASH(m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};

	SRV_Desc.Format = TextureDesc.Format;
	SRV_Desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SRV_Desc.TextureCube.MipLevels = 1;
	SRV_Desc.TextureCube.MostDetailedMip = 0;

	m_pDevice->CreateShaderResourceView(m_pTexture, &SRV_Desc, &m_pSRV);
	ASSERT_CRASH(m_pSRV);

	for (_uint i = 0; i < 6; ++i)
	{
		D3D11_RENDER_TARGET_VIEW_DESC RTV_Desc = {};

		RTV_Desc.Format = TextureDesc.Format;
		RTV_Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		RTV_Desc.Texture2DArray.MipSlice = 0;
		RTV_Desc.Texture2DArray.ArraySize = 1;
		RTV_Desc.Texture2DArray.FirstArraySlice = i;

		m_pDevice->CreateRenderTargetView(m_pTexture, &RTV_Desc, &m_pRTVs[i]);
		ASSERT_CRASH(m_pRTVs[i]);
	}


	//DSV
	D3D11_TEXTURE2D_DESC DepthTextureDesc = {};

	DepthTextureDesc.Width = g_iEnvMapSize;
	DepthTextureDesc.Height = g_iEnvMapSize;
	DepthTextureDesc.MipLevels = 1;
	DepthTextureDesc.ArraySize = 1;
	DepthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	DepthTextureDesc.SampleDesc.Quality = 0;
	DepthTextureDesc.SampleDesc.Count = 1;

	DepthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthTextureDesc.CPUAccessFlags = 0;
	DepthTextureDesc.MiscFlags = 0;

	ID3D11Texture2D* pDepthTexture = { nullptr };
	if (FAILED(m_pDevice->CreateTexture2D(&DepthTextureDesc, nullptr, &pDepthTexture)))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_Desc = {};
	DSV_Desc.Format = DepthTextureDesc.Format;
	DSV_Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_Desc.Texture2D.MipSlice = 0;

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthTexture, &DSV_Desc, &m_pDSV)))
		return E_FAIL;

	Safe_Release(pDepthTexture);

	return S_OK;
}

void CProbe::Setting_ProbeViewPort()
{
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.Width = static_cast<_float>(g_iEnvMapSize);
	Viewport.Height = static_cast<_float>(g_iEnvMapSize);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &Viewport);
}

void CProbe::Setting_PrevViewPort()
{
	m_pContext->RSSetViewports(1, &m_PrevViewPort);
}

HRESULT CProbe::Bind_ShaderResource(CShader* pShader, _uint iCubeFace)
{
	// Light PBR
	if (FAILED(pShader->Bind_Matrix("g_ViewMatrixInv", &m_MatricesInv[ENUM_CLASS(D3DTS::VIEW)][iCubeFace])))
		CRASH("Failed Bind ViewMatrixInv");

	if (FAILED(pShader->Bind_Matrix("g_ProjMatrixInv", &m_MatricesInv[ENUM_CLASS(D3DTS::PROJ)][iCubeFace])))
		CRASH("Failed Bind ProjMatrixInv");

	if (FAILED(pShader->Bind_Value("g_vCamPosition", &m_vCenter, sizeof(_float4))))
		CRASH("Failed Bind CamPosition");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_EnvDiffuse"), pShader, "g_DiffuseTexture")))
		CRASH("Failed Bind RT_EnvDiffuse");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_EnvNormal"), pShader, "g_NormalTexture")))
		CRASH("Failed Bind RT_EnvDiffuse");
	
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_EnvDepth"), pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_EnvDiffuse");
	
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_EnvPBR"), pShader, "g_PBRTexture")))
		CRASH("Failed Bind RT_EnvDiffuse");

	return S_OK;
}

void CProbe::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer_Rect, _uint iIndex)
{
	m_pContext->OMGetRenderTargets(1, &m_pBackBuffer, &m_pOriginDSV);

	_uint iNumViewPort = 1;
	m_pContext->RSGetViewports(&iNumViewPort, &m_PrevViewPort);

	Setting_ProbeViewPort();

	_float4 vClearColor = _float4(0.f, 0.f, 0.f, 0.f);
	for (_uint i = 0; i < 6; ++i)
	{
		m_pContext->ClearDepthStencilView(m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		m_pContext->ClearRenderTargetView(m_pRTVs[i], reinterpret_cast<_float*>(&vClearColor));

		m_pContext->OMSetRenderTargets(1, &m_pRTVs[i], m_pDSV);

		// SkyBox Render To ENV RTV
		Render_SkyBoxs(m_Matrices[ENUM_CLASS(D3DTS::VIEW)][i], m_Matrices[ENUM_CLASS(D3DTS::PROJ)][i]);

		// StaticObject Render To MRT EnvObject
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EnvObject"))))
			CRASH("Failed Begin MRT EnvObject");

		Render_StaticObjects(m_Matrices[ENUM_CLASS(D3DTS::VIEW)][i], m_Matrices[ENUM_CLASS(D3DTS::PROJ)][i]);

		m_pGameInstance->End_MRT();

		if (FAILED(Bind_ShaderResource(pShader, i)))
			return;

		m_pGameInstance->Render_LightEnvMap(pShader, pVIBuffer_Rect, m_pBounding);
	}

	m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pOriginDSV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginDSV);

	Setting_PrevViewPort();

	m_pBackBuffer = nullptr;
	m_pOriginDSV = nullptr;

	//ScratchImage CubeImage = {};
	//CaptureTexture(m_pDevice, m_pContext, m_pTexture, CubeImage);

	//_wstring strSaveName = TEXT("../../Client/Bin/Probe") + to_wstring(iIndex) + TEXT(".dds");
	//
	//SaveToDDSFile(CubeImage.GetImages(), CubeImage.GetImageCount(), CubeImage.GetMetadata(), DDS_FLAGS_NONE, strSaveName.c_str());

	m_SkyBoxs.clear();
	m_StaticObjects.clear();
}

void CProbe::Add_SkyBox(CGameObject* pSkyBox)
{
	ASSERT_CRASH(pSkyBox);

	m_SkyBoxs.push_back(pSkyBox);
}

void CProbe::Add_StaticObject(CStaticObject* pStaticObject)
{
	ASSERT_CRASH(pStaticObject);

	if (IsInProbe(pStaticObject->Get_BoundingBox()))
	{
		{
			lock_guard<mutex> lock(m_AddMutex);
			m_StaticObjects.push_back(pStaticObject);
		}
	}
}

CProbe* CProbe::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3& vCenter, const _float& fRange)
{
	CProbe* pInstance = new CProbe(pDevice, pContext);
	if (FAILED(pInstance->Initialize(vCenter, fRange)))
	{
		MSG_BOX("Failed to Created : CProbe");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CProbe::Free()
{
	__super::Free();

	Safe_Delete(m_pBounding);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pTexture);

	for(_uint i=0; i<6; ++i)
		Safe_Release(m_pRTVs[i]);

	Safe_Release(m_pDSV);
	Safe_Release(m_pSRV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginDSV);
}

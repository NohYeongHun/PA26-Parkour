#include "EnginePch.h"
#include "CSM.h"
#include "GameInstance.h"
#include "Shader.h"

CCSM::CCSM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CCSM::SetUp_ShadowLight(const _wstring& strLightTag)
{
	m_pLightDesc = m_pGameInstance->Get_LightDesc(strLightTag);
	
	ASSERT_CRASH(m_pLightDesc);
	
	return S_OK;
}

HRESULT CCSM::SetUp_ShadowNF()
{
	m_fCameraNear = m_pGameInstance->Get_CurrentCamera_Near();
	m_fCameraFar = m_pGameInstance->Get_CurrentCamera_Far();
	m_fCutFar = m_fCameraFar * 0.3f;

	for (_uint i = 0; i < m_iNumClipDistance; i++)
		m_fClipDistance[i] = Compute_ClipDistance(m_fCameraNear, m_fCutFar, i, m_iNumClip, 0.8f);

	return S_OK;
}

HRESULT CCSM::Initialize()
{
	m_iNumClip = g_iNumCascade;
	m_iNumClipDistance = m_iNumClip + 1;

	if (FAILED(Ready_CSM_View()))
		CRASH("Failed Created CSM");

	return S_OK;
}

void CCSM::Update_CSM()
{
	if (nullptr == m_pLightDesc)
		return;

	Update_Matrices();
}

void CCSM::Clear()
{
	m_pLightDesc = nullptr;
}

HRESULT CCSM::Bind_CSM_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName)
{
	ASSERT_CRASH(pShader);

	if (nullptr != pLightDirName && nullptr != m_pLightDesc)	// Renderer 호출용
	{
		if (FAILED(pShader->Bind_Value(pLightDirName, &m_pLightDesc->vDirection, sizeof(_float4))))
			CRASH("Failed Light Dir");
	}
	
	if (FAILED(pShader->Bind_Matrices(pViewName, &m_Matrices[ENUM_CLASS(D3DTS::VIEW)][0], m_iNumClip)))
		CRASH("Failed CSM View Matrices");
	
	if (FAILED(pShader->Bind_Matrices(pProjName, &m_Matrices[ENUM_CLASS(D3DTS::PROJ)][0], m_iNumClip)))
		CRASH("Failed CSM PROJ Matrices");

	return S_OK;
}

HRESULT CCSM::Bind_ShadowDistance_Resource(CShader* pShader, const _char* pDistanceName, const _char* pLastDistanceName)
{
	if (FAILED(pShader->Bind_Value(pDistanceName, &m_fClipDistance, sizeof(_float4))))
		CRASH("Failed Bind DistanceName");

	if (FAILED(pShader->Bind_Value(pLastDistanceName, &m_fClipDistance[4], sizeof(_float))))
		CRASH("Failed Bind DistanceName");

	return S_OK;
}


HRESULT CCSM::Bind_CSM_SRV(CShader* pShader, const _char* pConstantName)
{
	return pShader->Bind_Texture(pConstantName, m_pShadowSRV);
}

HRESULT CCSM::Begin_CSM()
{
	m_pContext->OMGetRenderTargets(1, &m_pBackBuffer, &m_pOriginalDSV);

	m_pContext->ClearDepthStencilView(m_pShadowDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	ID3D11RenderTargetView* pRTV = { nullptr };

	m_pContext->OMSetRenderTargets(1, &pRTV, m_pShadowDSV);

	return S_OK;
}

HRESULT CCSM::End_CSM()
{
	m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pOriginalDSV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);
	m_pBackBuffer = nullptr;
	m_pOriginalDSV = nullptr;

	return S_OK;
}

void CCSM::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	for (_uint i = 0; i < 4; i++)
	{
		_matrix World = XMMatrixScaling(150.f, 150.f, 1.f) * XMMatrixTranslationFromVector(XMVectorSet(-300.f, ( -200.f * (i +1) ) + 1280.f * 0.5f, 0.1f, 1.f));
		_float4x4 DebugWorld = {};
		XMStoreFloat4x4(&DebugWorld, World);

		pShader->Bind_Value("g_DebugCSMIndex", &i, sizeof(_uint));
		pShader->Bind_Matrix("g_WorldMatrix", &DebugWorld);

		pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::RD_DEBUG_CSM));
		pVIBuffer->Bind_Resources();
		pVIBuffer->Render();
	}
}

HRESULT CCSM::Ready_CSM_View()
{
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = g_iMaxWidth;
	TextureDesc.Height = g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = m_iNumClip;

	TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;			// DSV, SRV
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture2D* pTexture2D = { nullptr };
	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture2D)))
		CRASH("Shadow Texture");

	/////DSV/////
	D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
	DsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	DsvDesc.Texture2DArray.MipSlice = 0;
	DsvDesc.Texture2DArray.FirstArraySlice = 0;
	DsvDesc.Texture2DArray.ArraySize = m_iNumClip;

	if (FAILED(m_pDevice->CreateDepthStencilView(pTexture2D, &DsvDesc, &m_pShadowDSV)))
		CRASH("Shadow DSV");
	
	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	SrvDesc.Texture2DArray.MostDetailedMip = 0;
	SrvDesc.Texture2DArray.MipLevels = 1;
	SrvDesc.Texture2DArray.FirstArraySlice = 0;
	SrvDesc.Texture2DArray.ArraySize = m_iNumClip;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture2D, &SrvDesc, &m_pShadowSRV)))
		CRASH("Shadow SRV")

	Safe_Release(pTexture2D);

	return S_OK;
}

void CCSM::Update_Matrices()
{
	const _float4* pFrustrumWorldPoints = m_pGameInstance->Get_Frustrum_WorldPoints();
	
	Make_Matrices(pFrustrumWorldPoints);
}

void CCSM::Make_Matrices(const _float4* pFrustrumPoints)
{
	_float fClipNear = {};
	_float fClipFar = {};
	_float fNearRatio = {};
	_float fFarRatio = {};

	for (_uint j = 0; j < m_iNumClip; j++)
	{
		fClipNear = m_fClipDistance[j];
		fClipFar = m_fClipDistance[j + 1];
		
		fNearRatio = ( fClipNear - m_fCameraNear ) / (m_fCameraFar - m_fCameraNear );
		fFarRatio = ( fClipFar - m_fCameraNear ) / ( m_fCameraFar - m_fCameraNear );

		_float4 vClipPoints[8] = {};

		for (_uint k = 0; k < 4; k++)				// ?꾨윭?ㅽ? ?섎닠???덈줈???꾨윭?ㅽ? 留뚮뱾湲?
		{
			XMStoreFloat4(&vClipPoints[k], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fNearRatio));
			XMStoreFloat4(&vClipPoints[k + 4], XMVectorLerp(XMLoadFloat4(&pFrustrumPoints[k]), XMLoadFloat4(&pFrustrumPoints[k + 4]), fFarRatio));
		}

		XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)][j], Make_SplitViewMatrix(vClipPoints));
		XMStoreFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::PROJ)][j], Make_SplitProjMatrix(vClipPoints, XMLoadFloat4x4(&m_Matrices[ENUM_CLASS(D3DTS::VIEW)][j])));
	}

//	Make_ClipZ();
}

_vector CCSM::Compute_Center(const _float4* pFrustrumPoints)
{
	_vector vCenterPos = XMVectorZero();

	for (_uint i = 0; i < 8; i++)
		vCenterPos = XMVectorAdd(vCenterPos, XMLoadFloat4(&pFrustrumPoints[i]));

	vCenterPos = XMVectorSetW(XMVectorScale(vCenterPos, 0.125f), 1.f);

	return vCenterPos;
}

_float CCSM::Compute_Radius(const _float4* pFrustrumPoints, _vector vCenterPos )
{
	_float fRadius = 0.f;
	_float fDistance = 0.f;

	for (_uint i = 0; i < 8; i++)
	{
		fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vCenterPos, XMLoadFloat4(&pFrustrumPoints[i]))));
		fRadius = max(fRadius, fDistance);
	}

	return fRadius;
}

_float CCSM::Compute_ClipDistance(_float fNear, _float fFar, _uint iIndex, _uint iNumClip, _float fLambda)
{
	_float fClipDistance = {};
	_float fLinearClip = fNear + ( ( fFar - fNear ) * ( static_cast<_float>( iIndex ) / static_cast<_float>( iNumClip ) ) );	// Linear 蹂닿컙
	_float fLogClip = fNear * powf(fFar / fNear, static_cast<_float>( iIndex ) / static_cast<_float>( iNumClip ));				// Log	 蹂닿컙

	fClipDistance = ( fLambda * fLogClip ) + ( 1.f - fLambda ) * fLinearClip;			// Linear? Log Lerp

	return fClipDistance;
}

_matrix CCSM::Make_SplitViewMatrix(const _float4* pFrustrumPoints)
{
	_vector vCenterPos = Compute_Center(pFrustrumPoints);							// ���� ���������� �߽�

	_float	fMaxRadius = Compute_Radius(pFrustrumPoints, vCenterPos);				// �������� �������� ������������ �ִ� ����
	
	fMaxRadius += fMaxRadius * 0.1f;

	_vector vDir = XMLoadFloat4(&m_pLightDesc->vDirection);							// ���� ���� Light�� Dir

	_vector vEye = XMVectorSubtract(vCenterPos, XMVectorScale(vDir, fMaxRadius));	// Eye

	return XMMatrixLookAtLH(vEye, vCenterPos, XMVectorSet(0.f, 1.f, 0.f, 0.f));
}

_matrix CCSM::Make_SplitProjMatrix(const _float4* pFrustrumPoints, _fmatrix ShadowViewMatrix)
{
	_float4 vViewPoints[8] = {};

	_float fMinX = FLT_MAX, fMaxX = FLT_MAX * -1.f;
	_float fMinY = FLT_MAX, fMaxY = FLT_MAX * -1.f;
	_float fMinZ = FLT_MAX, fMaxZ = FLT_MAX * -1.f;

	for (_uint i = 0; i < 8; i++)
	{
		XMStoreFloat4(&vViewPoints[i], XMVector3TransformCoord(XMLoadFloat4(&pFrustrumPoints[i]), ShadowViewMatrix)); // 

		fMinX = min(fMinX, vViewPoints[i].x);
		fMaxX = max(fMaxX, vViewPoints[i].x);

		fMinY = min(fMinY, vViewPoints[i].y);
		fMaxY = max(fMaxY, vViewPoints[i].y);

		fMinZ = min(fMinZ, vViewPoints[i].z);
		fMaxZ = max(fMaxZ, vViewPoints[i].z);
	}

	_float fCascadeExtentX = (fMaxX - fMinX ) * 0.5f;				// Cascade Half Size X
	_float fCascadeExtentY = ( fMaxY - fMinY ) * 0.5f;				// Cascade Half Size Y 

	//_float fCascadeExtent = max(fCascadeExtentX, fCascadeExtentY) * 0.5f; // 

	fMinZ -= fMinZ * 0.1f;
	fMaxZ += fMaxZ * 0.1f;

	_float fTexelSizeX = ( fCascadeExtentX * 2.f) / static_cast<_float>(g_iMaxWidth);		// 현재 Cascade 프러스텀의 NDC상 Texel 사이즈 X

	_float fTexelSizeY = ( fCascadeExtentY * 2.f ) / static_cast<_float>(g_iMaxHeight);		// 현재 Cascade 프러스텀의 NDC상 Texel 사이즈 Y

	_float3 vViewCenterPos = {};
	XMStoreFloat3(&vViewCenterPos, Compute_Center(vViewPoints));

	vViewCenterPos.x = floor(vViewCenterPos.x / ( fTexelSizeX * 0.5f )) * fTexelSizeX;
	vViewCenterPos.y = floor(vViewCenterPos.y / ( fTexelSizeY * 0.5f )) * fTexelSizeY;

	_float4 vRect = _float4(vViewCenterPos.x - fCascadeExtentX,		//fCascadeExtent,
							vViewCenterPos.x + fCascadeExtentX,		//fCascadeExtent,
							vViewCenterPos.y - fCascadeExtentY,		//fCascadeExtent,
							vViewCenterPos.y + fCascadeExtentY);	// fCascadeExtent);

	_float fCascadeExtentZ = ( fMaxZ - fMinZ ) * 0.5f;

	_float fNear = vViewCenterPos.z - fCascadeExtentZ;
	_float fFar = vViewCenterPos.z + fCascadeExtentZ;

	return XMMatrixOrthographicOffCenterLH(vRect.x, vRect.y, vRect.z, vRect.w, fNear, fFar);
}

CCSM* CCSM::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCSM* pInstance = new CCSM(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CCSM");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCSM::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	Safe_Release(m_pShadowDSV);
	Safe_Release(m_pShadowSRV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);
}

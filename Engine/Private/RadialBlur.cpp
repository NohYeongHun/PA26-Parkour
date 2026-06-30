#include "EnginePch.h"
#include "RadialBlur.h"
#include "GameInstance.h"

CRadialBlur::CRadialBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContrext)
	: CBlur { pDevice, pContrext }
{
}

HRESULT CRadialBlur::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;
	
	m_RadialData.vPivot = _float2(0.5f, 0.5f);
	m_RadialData.fMinDistance = 0.0f;
	m_RadialData.fMaxDistance = 0.1f;
	m_RadialData.fLengthScale = -0.2f;

	m_fTargetLength = m_RadialData.fLengthScale;

	D3D11_SAMPLER_DESC ClampSamplerDesc = {};
	ClampSamplerDesc.Filter = D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
	ClampSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ClampSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ClampSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ClampSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	ClampSamplerDesc.MinLOD = 0;
	ClampSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&ClampSamplerDesc, &m_pClampSampler);
	ASSERT_CRASH(m_pClampSampler);

	return S_OK;
}

void CRadialBlur::Update(_float fTimeDelta)
{
	m_fCurLength = lerp(0.f, m_fTargetLength, m_fIntensity);

	m_RadialData.fLengthScale = m_fCurLength;
}

HRESULT CRadialBlur::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	_uint iDownSizeX = m_iWinSizeX >> 1;
	_uint iDownSizeY = m_iWinSizeY >> 1;

	//BackBuffer DownScale
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_CurrentSceneSRV())))//m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOWNSAMPLE");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_RadialBlur"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_RadialBlur"), "RADIAL_DATA", &m_RadialData, sizeof(RADIAL_DATA))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_SamplerState(TEXT("RCS_RadialBlur"), 0, m_pClampSampler)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_RadialBlur"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_MotionBlur");

	//Combined
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Failed Begin MRT_BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed Bind BackBuffer");
	//if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Combine"), pShader, "g_BackBufferTexture")))
	//	CRASH("Failed Bind BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_RadialBlur")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(pShader->Bind_Value("g_fEffectIntensity", &m_fIntensity, sizeof(_float))))
		CRASH("Failed Bind g_fEffectIntensity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();
	
	m_pGameInstance->End_MRT();

	return S_OK;
}

void CRadialBlur::Enter()
{
}

void CRadialBlur::Exit()
{
}

void CRadialBlur::Setting_Radial(_float2 vCenterUV, _float2 vDistanceRange, _float fRadialIntensity)
{
	m_RadialData.vPivot = vCenterUV;
	m_RadialData.fMinDistance = vDistanceRange.x;
	m_RadialData.fMaxDistance = vDistanceRange.y;
	m_RadialData.fLengthScale = fRadialIntensity;

	m_fTargetLength = m_RadialData.fLengthScale;
}

void CRadialBlur::Setting_Radial(_fvector vCenterPos, _float2 vDistanceRange, _float fRadialIntensity)
{
	_matrix ViewMatrix = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_TransformState_Matrix(D3DTS::PROJ);

	_vector vProjPos = XMVector3TransformCoord(XMVector3TransformCoord(vCenterPos, ViewMatrix), ProjMatrix);

	vProjPos = XMVectorScale(vProjPos, 1.f / XMVectorGetW(vProjPos));

	_float fU = XMVectorGetX(vProjPos) * 0.5f + 0.5f;
	_float fV = XMVectorGetY(vProjPos) * -0.5f + 0.5f;
	
	m_RadialData.vPivot = _float2(fU, fV);
	m_RadialData.fMinDistance = vDistanceRange.x;
	m_RadialData.fMaxDistance = vDistanceRange.y;
	m_RadialData.fLengthScale = fRadialIntensity;
}

CRadialBlur* CRadialBlur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CRadialBlur* pInstance = new CRadialBlur(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CRadialBlur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRadialBlur::Free()
{
	__super::Free();

	Safe_Release(m_pClampSampler);
}

#include "EnginePch.h"
#include "MotionBlur.h"
#include "GameInstance.h"

CMotionBlur::CMotionBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlur { pDevice, pContext }
{
}

HRESULT CMotionBlur::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_fWinSizeX = static_cast<_float>(iWinSizeX);
	m_fWinSizeY = static_cast<_float>(iWinSizeY);

	m_MotionBlurData.fLimitVelocity = 2.f;
	m_MotionBlurData.fLimitDepth = 50.f;
	m_MotionBlurData.fLengthScale = 0.5f;
	m_MotionBlurData.fSampleDepthBias = 10.f;

	m_fTargetLength = m_MotionBlurData.fLengthScale;

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

void CMotionBlur::Update(_float fTimeDelta)
{
	m_fCurLength = lerp(0.f, m_fTargetLength, m_fIntensity);

	m_MotionBlurData.fLengthScale = m_fCurLength;
}

HRESULT CMotionBlur::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	//READY VELOCITY_MAP
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_VELOCITY_MAP"))))
		CRASH("Failed Begin MRT_VELOCITY_MAP");

	if (FAILED(pShader->Bind_Value("g_fWidth", (&m_fWinSizeX), sizeof(_float))))
		CRASH("Failed Bind g_fWidth");
	if (FAILED(pShader->Bind_Value("g_fHeight", (&m_fWinSizeY), sizeof(_float))))
		CRASH("Failed Bind g_fHeight");

	if (FAILED(pShader->Bind_Matrix("g_PrevCamViewMatrix", m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_PrevCamProjMatrix", m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");

	if (FAILED(pShader->Bind_Value("g_fLimitDepth", &m_MotionBlurData.fLimitDepth, sizeof(_float))))
		CRASH("Failed Bind g_fLimitVelocity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::VELOCITY_MAP));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	//// DOWNSAMPLE
	//_uint iDownSizeX = m_iWinSizeX >> 1;
	//_uint iDownSizeY = m_iWinSizeY >> 1;

	////BackBuffer DownScale
	//if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", m_pGameInstance->Get_CurrentSceneSRV())))//m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine")))))
	//	CRASH("Failed Add_SRVData");

	//if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY)))
	//	CRASH("Failed RCS_DOWNSAMPLE");

	//// DEPTH DownScale
	//if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE_DEPTH"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
	//	CRASH("Failed Add_SRVData");

	//if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"), iDownSizeX, iDownSizeY)))
	//	CRASH("Failed RCS_DOWNSAMPLE");

	// Motion Blur + UpScale
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "InputTexture", m_pGameInstance->Get_CurrentSceneSRV())))//m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))//m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_MotionBlur"), "VelocityMap", m_pGameInstance->Get_RT_SRV(TEXT("RT_VelocityMap")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_MotionBlur"), "MOTION_DATA", reinterpret_cast<void*>(&m_MotionBlurData), sizeof(MOTION_BLUR_DATA))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_SamplerState(TEXT("RCS_MotionBlur"), 0, m_pClampSampler)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_MotionBlur"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_MotionBlur");

	//Combined
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Failed Begin MRT_BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed Bind BackBuffer");
	
	if (FAILED(pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_MotionBlur")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(pShader->Bind_Texture("g_VelocityMap", m_pGameInstance->Get_RT_SRV(TEXT("RT_VelocityMap")))))
		CRASH("Failed Bind VelocityMap");

	if (FAILED(pShader->Bind_Value("g_fLimitVelocity", &m_MotionBlurData.fLimitVelocity, sizeof(_float))))
		CRASH("Failed Bind g_fLimitVelocity");

	if (FAILED(pShader->Bind_Value("g_fEffectIntensity", &m_fIntensity, sizeof(_float))))
		CRASH("Failed Bind g_fEffectIntensity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::MOTION_BLUR));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

void CMotionBlur::Enter()
{
}

void CMotionBlur::Exit()
{
}

CMotionBlur* CMotionBlur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CMotionBlur* pInstance = new CMotionBlur(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CMotionBlur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMotionBlur::Free()
{
	__super::Free();

	Safe_Release(m_pClampSampler);
}

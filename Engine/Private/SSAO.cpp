#include "EnginePch.h"
#include "SSAO.h"
#include "GameInstance.h"

CSSAO::CSSAO(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSFX { pDevice, pContext }
{
}

HRESULT CSSAO::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_fWinSizeX = static_cast<_float>(iWinSizeX);
	m_fWinSizeY = static_cast<_float>(iWinSizeY);

	//SSAO
	m_iNumKernel = 16;
	m_fRadius = 1.f;
	m_fMaxDistance = 0.5f;
	m_fOutDistance = 500.f;

	// SSAO_Blur
	m_fSSAO_MinDepthDistance = 5.f;

	if (FAILED(Ready_NoiseTexture()))
		return E_FAIL;

	if (FAILED(Ready_SSAO_SampleVector()))
		return E_FAIL;

    return S_OK;
}

HRESULT CSSAO::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
#pragma region SSAO
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SSAO"))))
		CRASH("Render Fail");

	if (FAILED(Bind_Resources(pShader)))
		CRASH("Failed Bind SSAO Resources");

	if (FAILED(pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::SSAO))))
		CRASH("Render Fail");

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();
#pragma endregion

#pragma region BLUR_X
	SSAO_BLUR_DATA Data = {};
	Data.fSSAO_MinDepthDistance = m_fSSAO_MinDepthDistance;
	Data.fWidth = m_fWinSizeX;
	Data.fHeight = m_fWinSizeY;

	if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_SSAO_BLUR_X"), "SSAO_BLUR_DATA", reinterpret_cast<void*>(&Data), sizeof(SSAO_BLUR_DATA))))
		return E_FAIL;

	//if (FAILED(m_pSubResource->Add_SSAO_Blur_BufferData(TEXT("RCS_SSAO_BLUR_X"), m_fWinSizeX, m_fWinSizeY)))
	//	CRASH("Failed Add_SSAO_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_SSAO")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_X"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSAO_BLUR_X"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_SSAO_BLUR_X");
#pragma endregion

#pragma region BLUR_Y
	if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_SSAO_BLUR_Y"), "SSAO_BLUR_DATA", reinterpret_cast<void*>(&Data), sizeof(SSAO_BLUR_DATA))))
		return E_FAIL;
	//if (FAILED(m_pSubResource->Add_SSAO_Blur_BufferData(TEXT("RCS_SSAO_BLUR_Y"), m_fWinSizeX, m_fWinSizeY)))
	//	CRASH("Failed Add_SSAO_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_SSAO_BLUR_X")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSAO_BLUR_Y"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSAO_BLUR_Y"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_SSAO_BLUR_Y");
#pragma endregion

    return S_OK;
}

HRESULT CSSAO::Bind_Resources(CShader* pShader)
{
	if (FAILED(pShader->Bind_Matrix("g_CamViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Render Fail");

	if (FAILED(pShader->Bind_Matrix("g_CamProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Render Fail");

	if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(pShader, "g_NoiseTexture")))
		CRASH("Failed Bind NoiseTexture");

	if (FAILED(pShader->Bind_Value("g_vSampleVector", m_SSAO_SampleVector.data(), sizeof(_float4) * m_iNumKernel)))
		CRASH("Failed Bind Sample Vector");

	if (FAILED(pShader->Bind_Value("g_iSampleSize", &m_iNumKernel, sizeof(_uint))))
		CRASH("Failed Bind g_iSampleSize");

	if (FAILED(pShader->Bind_Value("g_fSSAO_Radius", &m_fRadius, sizeof(_float))))
		CRASH("Failed Bind g_fSSAO_Radius");

	if (FAILED(pShader->Bind_Value("g_fSSAO_MaxDistance", &m_fMaxDistance, sizeof(_float))))
		CRASH("Failed Bind g_fSSAO_MaxDistance");

	if (FAILED(pShader->Bind_Value("g_fSSAO_OutDistance", &m_fOutDistance, sizeof(_float))))
		CRASH("Failed Bind g_fSSAO_OutDistance");

	return S_OK;
}

HRESULT CSSAO::Ready_NoiseTexture()
{
	m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/Resource/SSAO_Noise.png"), 1);
	ASSERT_CRASH(m_pNoiseTexture);

	return S_OK;
}

HRESULT CSSAO::Ready_SSAO_SampleVector()
{
	for (_uint i = 0; i < m_iNumKernel; i++)
	{
		_float fX = m_pGameInstance->Rand(-1.f, 1.f);
		_float fY = m_pGameInstance->Rand(-1.f, 1.f);
		_float fZ = m_pGameInstance->Rand_Normal();

		_vector vSample = XMVector3Normalize(XMVectorSet(fX, fY, fZ, 0.f));

		_float fScale = static_cast<_float>(i) / static_cast<_float>(m_iNumKernel);
		fScale = 0.1f + (0.9f * pow(fScale, 2));

		m_SSAO_SampleVector.push_back(XMVectorScale(vSample, fScale));
	}

	return S_OK;
}

CSSAO* CSSAO::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CSSAO* pInstance = new CSSAO(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CSSAO");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CSSAO::Free()
{
	__super::Free();

	Safe_Release(m_pNoiseTexture);
}

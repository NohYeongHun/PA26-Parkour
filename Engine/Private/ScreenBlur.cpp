#include "EnginePch.h"
#include "ScreenBlur.h"
#include "GameInstance.h"

CScreenBlur::CScreenBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlur { pDevice, pContext }
{
}

HRESULT CScreenBlur::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

HRESULT CScreenBlur::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	for (_uint i = 0; i <= 1; i++)
	{
		ID3D11ShaderResourceView* pDown = i == 0 ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine")) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), i - 1);

		_uint iDownSizeX = m_iWinSizeX >> (i + 1);
		_uint iDownSizeY = m_iWinSizeY >> (i + 1);

		// DOWNSAMPLE SECOND
		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", pDown)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY, i)))
			CRASH("Failed RCS_DOWNSAMPLE");
	}

	_uint iBlurSizeX = m_iWinSizeX >> 2;
	_uint iBlurSizeY = m_iWinSizeY >> 2;

	_uint iBlurWeight = 0;

	if (FAILED(__super::Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_X"), iBlurSizeX, iBlurSizeY, iBlurWeight)))
		CRASH("Failed Add_SizeData_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_X"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), 1))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_X"), iBlurSizeX, iBlurSizeY, 1)))
		CRASH("Failed RCS_GAUSSIAN_BLUR_X");

	// BLUR_Y
	if (FAILED(__super::Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_Y"), iBlurSizeX, iBlurSizeY, iBlurWeight)))
		CRASH("Failed Add_SizeData_BufferData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_X"), 1))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_Y"), iBlurSizeX, iBlurSizeY, 1)))
		CRASH("Failed RCS_GAUSSIAN_BLUR_Y");

	for (_int j = 1; j >= 0; --j)
	{
		// UPSAMPLE
		_uint iUpWinSizeX = (m_iWinSizeX) >> j;
		_uint iUpWinSizeY = (m_iWinSizeY) >> j;

		ID3D11ShaderResourceView* pUp = j == 1 ? m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_Y"), j) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE"), j + 1);

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE"), "InputTexture", pUp)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_UPSAMPLE"), iUpWinSizeX, iUpWinSizeY, j)))
			CRASH("Failed RCS_UPSAMPLE");
	}

	//Combined
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Failed Begin MRT_BackBuffer");

	if(FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed Bind BackBuffer");

//	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Combine"), pShader, "g_BackBufferTexture")))
//		CRASH("Failed Bind BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE")))))
		CRASH("Failed Bind Blur Texture");

	
	if (FAILED(pShader->Bind_Value("g_fEffectIntensity", &m_fIntensity, sizeof(_float))))
		CRASH("Failed Bind g_fEffectIntensity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLUR));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

CScreenBlur* CScreenBlur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CScreenBlur* pInstance = new CScreenBlur(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CScreenBlur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CScreenBlur::Free()
{
	__super::Free();
}

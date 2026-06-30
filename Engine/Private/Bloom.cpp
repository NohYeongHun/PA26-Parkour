#include "EnginePch.h"
#include "Bloom.h"
#include "GameInstance.h"

CBloom::CBloom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlur { pDevice, pContext }
{
}

HRESULT CBloom::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_iBloomWeight = 1;

	m_fBoolIntensity = 0.25f;

    return S_OK;
}

HRESULT CBloom::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	// DOWNSAMPLE
	for (_uint i = 0; i <= 2; i++)
	{
		ID3D11ShaderResourceView* pDown = i == 0 ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Emissive")) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), i - 1);

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture", pDown)))
			CRASH("Failed Add_SRVData");

		_uint iDownSizeX = m_iWinSizeX >> (i + 1);
		_uint iDownSizeY = m_iWinSizeY >> (i + 1);

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY, i)))
			CRASH("Failed RCS_DOWNSAMPLE");
	}

	for (_int j = 2; j >= 0; --j)
	{
		// BLUR_X
		_uint iBlurSizeX = m_iWinSizeX >> (j + 1);
		_uint iBlurSizeY = m_iWinSizeY >> (j + 1);

		if (FAILED(__super::Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_X"), iBlurSizeX, iBlurSizeY, m_iBloomWeight)))
			CRASH("Failed Add_SizeData_BufferData");

		ID3D11ShaderResourceView* pBlur = j == 2 ? m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), j) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE_BLOOM"), j + 1);

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_X"), "InputTexture", pBlur)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_X"), iBlurSizeX, iBlurSizeY, j)))
			CRASH("Failed RCS_GAUSSIAN_BLUR_X");

		// BLUR_Y
		if (FAILED(__super::Add_Blur_BufferData(TEXT("RCS_GAUSSIAN_BLUR_Y"), iBlurSizeX, iBlurSizeY, m_iBloomWeight)))
			CRASH("Failed Add_SizeData_BufferData");

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_GAUSSIAN_BLUR_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_X"), j))))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_GAUSSIAN_BLUR_Y"), iBlurSizeX, iBlurSizeY, j)))
			CRASH("Failed RCS_GAUSSIAN_BLUR_Y");

		// UPSAMPLE
		_uint iUpSizeX = (m_iWinSizeX) >> j;
		_uint iUpSizeY = (m_iWinSizeY) >> j;

		BLOOM_UP_DATA Data = {};
		Data.vSize = _float2(iUpSizeX, iUpSizeY);
		Data.fIntensity = (1.f - static_cast<_float>((j + 1)) * m_fBoolIntensity);

		if (FAILED(m_pGameInstance->Add_BufferData(TEXT("RCS_UPSAMPLE_BLOOM"), "BLOOM_DATA", reinterpret_cast<void*>(&Data), sizeof(BLOOM_UP_DATA))))
			return E_FAIL;

		//if (FAILED(m_pSubResource->Add_Bloom_BufferData(TEXT("RCS_UPSAMPLE_BLOOM"), iUpSizeX, iUpSizeY, j)))
		//	CRASH("Failed Add_UpSample_BufferData");

		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE_BLOOM"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_GAUSSIAN_BLUR_Y"), j))))
			CRASH("Failed Add_SRVData");

		ID3D11ShaderResourceView* pBase = j == 0 ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Emissive")) : m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE"), j - 1);
		if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE_BLOOM"), "BaseTexture", pBase)))
			CRASH("Failed Add_SRVData");

		if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_UPSAMPLE_BLOOM"), iUpSizeX, iUpSizeY, j)))
			CRASH("Failed RCS_UPSAMPLE");
	}

    return S_OK;
}

CBloom* CBloom::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CBloom* pInstance = new CBloom(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CBloom");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CBloom::Free()
{
	__super::Free();
}

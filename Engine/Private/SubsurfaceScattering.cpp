#include "EnginePch.h"
#include "SubsurfaceScattering.h"
#include "GameInstance.h"

CSubsurfaceScattering::CSubsurfaceScattering(ID3D11Device* pDevice, ID3D11DeviceContext* pContrext)
	: CBlur { pDevice, pContrext }
{
}

HRESULT CSubsurfaceScattering::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{


	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

void CSubsurfaceScattering::Update(_float fTimeDelta)
{
}

HRESULT CSubsurfaceScattering::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	//BLUR X
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_X"), "g_DiffuseTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_LightDiffuse")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_X"), "g_StrengthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_SSS")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_X"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_X"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSSBlur_X"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_GAUSSIAN_BLUR_X");

	//BLUR Y
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_Y"), "g_DiffuseTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_SSSBlur_X")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_Y"), "g_StrengthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_SSS")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_Y"), "g_NormalTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Normal")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_SSSBlur_Y"), "g_DepthTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_SSSBlur_Y"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_GAUSSIAN_BLUR_X");

	return S_OK;
}

CSubsurfaceScattering* CSubsurfaceScattering::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CSubsurfaceScattering* pInstance = new CSubsurfaceScattering(pDevice, pContext);
	if(FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CSubsurfaceScattering");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSubsurfaceScattering::Free()
{
	__super::Free();
}

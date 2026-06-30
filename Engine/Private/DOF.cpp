#include "EnginePch.h"
#include "DOF.h"
#include "GameInstance.h"

CDOF::CDOF(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlur { pDevice, pContext }
{
}

HRESULT CDOF::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_fDofRange = 100.f;
	m_fDofScale = 0.3f;

    return S_OK;
}

HRESULT CDOF::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	//COC 계산
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DOF"))))
		CRASH("Failed Begin MRT_DOF");

	if (FAILED(Bind_Resources(pShader)))
		CRASH("Failed Bind_Dof_Resource");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DOF_DEPTH));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	_uint iDownSizeX = m_iWinSizeX >> 1;
	_uint iDownSizeY = m_iWinSizeY >> 1;

	//DOWNSAMPLE
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE"), "InputTexture",  m_pGameInstance->Get_CurrentSceneSRV())))//m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOWNSAMPLE");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOWNSAMPLE_DEPTH"), "InputTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Dof")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOWNSAMPLE");

	//BLUR_X
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_X"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_X"), "DepthTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOF_X"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOF_X");

	//BLUR_Y
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_Y"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOF_X")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_DOF_Y"), "DepthTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOWNSAMPLE_DEPTH")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_DOF_Y"), iDownSizeX, iDownSizeY)))
		CRASH("Failed RCS_DOF_Y");

	//UPSAMPLE
	if (FAILED(m_pGameInstance->Add_SRVData(TEXT("RCS_UPSAMPLE"), "InputTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_DOF_Y")))))
		CRASH("Failed Add_SRVData");

	if (FAILED(m_pGameInstance->Begin_RCS(TEXT("RCS_UPSAMPLE"), m_iWinSizeX, m_iWinSizeY)))
		CRASH("Failed RCS_UPSAMPLE");

	//Combined
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Failed Begin MRT_BackBuffer");

	if(FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed Bind BackBuffer");
	//if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Combine"), pShader, "g_BackBufferTexture")))
	//	CRASH("Failed Bind BackBuffer");

	if (FAILED(pShader->Bind_Texture("g_DofTexture", m_pGameInstance->Get_RT_SRV(TEXT("RT_Dof")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(pShader->Bind_Texture("g_BlurTexture", m_pGameInstance->Get_RCS_SRV(TEXT("RCS_UPSAMPLE")))))
		CRASH("Failed Bind Blur Texture");

	if (FAILED(pShader->Bind_Value("g_fEffectIntensity", &m_fIntensity, sizeof(_float))))
		CRASH("Failed Bind g_fEffectIntensity");

	pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DOF));

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	return S_OK;
}

void CDOF::Setting_DOF(_float3 vCenterPos, _float fRange)
{
	m_vCenterPos = vCenterPos;
	m_fDofRange = fRange;
}

HRESULT CDOF::Bind_Resources(CShader* pShader)
{
	if (FAILED(pShader->Bind_Value("g_vFocusPos", &m_vCenterPos, sizeof(_float3))))
		CRASH("Failed Bind Fog Distance");
	if (FAILED(pShader->Bind_Value("g_fFocusRange", &m_fDofRange, sizeof(_float))))
		CRASH("Failed Bind Fog Distance");
	if (FAILED(pShader->Bind_Value("g_fFocusMinCoc", &m_fDofScale, sizeof(_float))))
		CRASH("Failed Bind Fog Distance");

	return S_OK;
}

CDOF* CDOF::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CDOF* pInstance = new CDOF(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CDOF");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CDOF::Free()
{
	__super::Free();
}

#include "EnginePch.h"
#include "SFX_Hub.h"
#include "SFX.h"
#include "GameInstance.h"
#include "RendererCS.h"
#include "SSAO.h"
#include "Bloom.h"
#include "DOF.h"
#include "MotionBlur.h"
#include "ScreenBlur.h"
#include "RadialBlur.h"
#include "SubsurfaceScattering.h"
#include "Water.h"

CSFX_Hub::CSFX_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice}
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CSFX_Hub::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	m_fDefaultIntensityBoost = 2.f;

	if (FAILED(Ready_SFX()))
		return E_FAIL;

	if (FAILED(Ready_SFX_CS()))
		return E_FAIL;

	return S_OK;
}

void CSFX_Hub::Update_SFX(_float fTimeDelta)
{
	if (nullptr == m_pCurrentSFX)
		return;

	m_pCurrentSFX->Update(fTimeDelta);
	Update_Toggle(fTimeDelta);
	Update_ToggleIntensity(fTimeDelta);
}

HRESULT CSFX_Hub::Begin_Toggle_SFX(SFX_TOGGLE eType, _float fDuration)
{
	if (m_eCurrentToggle == eType && true == m_IsToggleOn)
		return S_OK;

	CSFX* pSFX = Find_SFX(static_cast<SFX_TYPE>(eType));
	if (nullptr == pSFX)
		return E_FAIL;

	if (nullptr != m_pCurrentSFX)
	{
		m_pCurrentSFX->Exit();
		Safe_Release(m_pCurrentSFX);
	}

	m_IsToggleOn = true;
	
	m_fToggleDuration = fDuration;

	m_fIntensityBoost = m_fToggleDuration == 0.f ? m_fDefaultIntensityBoost : min((1.f / (m_fToggleDuration * 0.2f)), m_fDefaultIntensityBoost);

	m_fToggleIntensity = 0.f;

	m_pCurrentSFX = pSFX;
	m_pCurrentSFX->Enter();
	m_eCurrentToggle = eType;

	Safe_AddRef(m_pCurrentSFX);

	return S_OK;
}

HRESULT CSFX_Hub::End_SFX()
{
	if (nullptr == m_pCurrentSFX)
		return S_OK;

	m_IsToggleOn = false;
	m_fToggleDuration = 0.f;
	m_fCurrentToggleDuration = 0.f;

	return S_OK;
}

HRESULT CSFX_Hub::Render_SFX_Toggle(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	if (nullptr == m_pCurrentSFX)
		return E_FAIL;

	m_pCurrentSFX->Set_Intensity(m_fToggleIntensity);

	m_pCurrentSFX->Render(pVIBuffer, pShader);

	return S_OK;
}

HRESULT CSFX_Hub::Render_SFX(SFX_TYPE eType, CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	CSFX* pSFX = Find_SFX(eType);
	if (nullptr == pSFX)
		return E_FAIL;

	return pSFX->Render(pVIBuffer, pShader);
}

HRESULT CSFX_Hub::Setting_DOF(_float3 vCenterPos, _float fRange)
{
	CSFX* pSFX = Find_SFX(SFX_TYPE::DOF);
	ASSERT_CRASH(pSFX);

	CDOF* pDOF = static_cast<CDOF*>(pSFX);
	pDOF->Setting_DOF(vCenterPos, fRange);

	return S_OK;
}

HRESULT CSFX_Hub::Setting_Radial(_float2 vCenterUV, _float2 vDistanceRange, _float fRadialIntensity)
{
	CSFX* pSFX = Find_SFX(SFX_TYPE::RADIAL);
	ASSERT_CRASH(pSFX);

	CRadialBlur* pRadial = static_cast<CRadialBlur*>(pSFX);
	pRadial->Setting_Radial(vCenterUV, vDistanceRange, fRadialIntensity);

	return S_OK;
}

HRESULT CSFX_Hub::Setting_Radial(_fvector vCenterPos, _float2 vDistanceRange, _float fRadialIntensity)
{
	CSFX* pSFX = Find_SFX(SFX_TYPE::RADIAL);
	ASSERT_CRASH(pSFX);

	CRadialBlur* pRadial = static_cast<CRadialBlur*>(pSFX);
	pRadial->Setting_Radial(vCenterPos, vDistanceRange, fRadialIntensity);

	return S_OK;
}

#ifdef _DEBUG
void CSFX_Hub::Set_Motion(_float fLimitVelocity, _float fLimitDepth, _float fLengthScale)
{
	CMotionBlur* pSFX = static_cast<CMotionBlur*>(Find_SFX(SFX_TYPE::MOTION));
	pSFX->Set_Motion(fLimitVelocity, fLimitDepth, fLengthScale);
}
void CSFX_Hub::Set_SSR(_float fMinStep, _float fMaxStep, _float fStartOffset)
{
	CWater* pWater = static_cast<CWater*>(Find_SFX(SFX_TYPE::WATER));
	pWater->Set_SSR(fMinStep, fMaxStep, fStartOffset);
}
#endif

CSFX* CSFX_Hub::Find_SFX(SFX_TYPE eType)
{
	auto iter = m_SFXs.find(eType);
	if (iter == m_SFXs.end())
		return nullptr;
	return iter->second;
}

HRESULT CSFX_Hub::Ready_SFX()
{
	CSSAO* pSSao = CSSAO::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pSSao);
	m_SFXs.emplace(SFX_TYPE::SSAO, pSSao);

	CBloom* pBloom = CBloom::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pSSao);
	m_SFXs.emplace(SFX_TYPE::BLOOM, pBloom);

	CMotionBlur* pMotionBlur = CMotionBlur::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pMotionBlur);
	m_SFXs.emplace(SFX_TYPE::MOTION, pMotionBlur);

	CDOF* pDOF = CDOF::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pDOF);
	m_SFXs.emplace(SFX_TYPE::DOF, pDOF);

	CScreenBlur* pScreenBlur = CScreenBlur::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pScreenBlur);
	m_SFXs.emplace(SFX_TYPE::BLUR, pScreenBlur);

	CRadialBlur* pRadialBlur = CRadialBlur::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pRadialBlur);
	m_SFXs.emplace(SFX_TYPE::RADIAL, pRadialBlur);

	CSubsurfaceScattering* pSSS = CSubsurfaceScattering::Create(m_pDevice, m_pContext, m_iWinSizeX, m_iWinSizeY);
	ASSERT_CRASH(pSSS);
	m_SFXs.emplace(SFX_TYPE::SSS, pSSS);

	CWater* pWater = CWater::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(pWater);
	m_SFXs.emplace(SFX_TYPE::WATER, pWater);

	return S_OK;
}

HRESULT CSFX_Hub::Ready_SFX_CS()
{
#pragma region SSAO_BLUR_RCS
	CRendererCS::RCS_DESC BlurDesc = {};
	BlurDesc.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_SSAO.hlsl");
	BlurDesc.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	BlurDesc.strEntryPoint = "SSAO_BLUR_X";
	BlurDesc.iWidth = m_iWinSizeX;
	BlurDesc.iHeight = m_iWinSizeY;
	BlurDesc.fDefinitionX = 16.f;
	BlurDesc.fDefinitionY = 16.f;
	BlurDesc.eFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	BlurDesc.iMipLevels = 1;
	BlurDesc.vClearColor = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSAO_BLUR_X"), &BlurDesc)))
		CRASH("Failed Add RCS_SSAO");

	BlurDesc.strEntryPoint = "SSAO_BLUR_Y";

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSAO_BLUR_Y"), &BlurDesc)))
		CRASH("Failed Add RCS_SSAO");

#pragma endregion

#pragma region BLUR
	CRendererCS::RCS_DESC BlurRCS = {};
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Blur.hlsl");
	BlurRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	BlurRCS.strEntryPoint = "GaussianBlur_X";
	BlurRCS.iWidth = m_iWinSizeX >> 1;
	BlurRCS.iHeight = m_iWinSizeY >> 1;
	BlurRCS.fDefinitionX = 16.f;
	BlurRCS.fDefinitionY = 16.f;
	BlurRCS.eFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	BlurRCS.iMipLevels = 3;
	BlurRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_GAUSSIAN_BLUR_X"), &BlurRCS)))
		CRASH("Failed Add G_BlurX");
	 
	BlurRCS.strEntryPoint = "GaussianBlur_Y";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_GAUSSIAN_BLUR_Y"), &BlurRCS)))
		CRASH("Failed Add G_BlurY");

#pragma region DOF
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_DOF.hlsl");
	BlurRCS.strEntryPoint = "DOF_X";
	BlurRCS.iMipLevels = 2;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOF_X"), &BlurRCS)))
		CRASH("Failed Add DOF_X");

	BlurRCS.strEntryPoint = "DOF_Y";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOF_Y"), &BlurRCS)))
		CRASH("Failed Add DOF_Y");
#pragma endregion

#pragma region MOTION_BLUR
	// Blur + UpSample
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_MotionBlur.hlsl");
	BlurRCS.strEntryPoint = "Motion_Blur";
	BlurRCS.iWidth = m_iWinSizeX;
	BlurRCS.iHeight = m_iWinSizeY;
	BlurRCS.iMipLevels = 1;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_MotionBlur"), &BlurRCS)))
		CRASH("Failed Add RCS_MotionBlur");
#pragma endregion

#pragma region RADIAL_BLUR
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_RadialBlur.hlsl");
	BlurRCS.strEntryPoint = "RadialBlur";
	BlurRCS.iWidth = m_iWinSizeX;
	BlurRCS.iHeight = m_iWinSizeY;
	BlurRCS.iMipLevels = 1;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_RadialBlur"), &BlurRCS)))
		CRASH("Failed Add RCS_MotionBlur");
#pragma endregion

#pragma region SSS
	BlurRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_SSS.hlsl");
	BlurRCS.strEntryPoint = "SSSBlur_X";
	BlurRCS.iWidth = m_iWinSizeX;
	BlurRCS.iHeight = m_iWinSizeY;
	BlurRCS.iMipLevels = 1;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSSBlur_X"), &BlurRCS)))
		CRASH("Failed Add RCS_SSSBlur_X");

	BlurRCS.strEntryPoint = "SSSBlur_Y";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_SSSBlur_Y"), &BlurRCS)))
		CRASH("Failed Add RCS_SSSBlur_Y");
#pragma endregion

#pragma region DOWNSAMPLE
	CRendererCS::RCS_DESC DownSampleRCS = {};
	DownSampleRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Sampling.hlsl");
	DownSampleRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	DownSampleRCS.strEntryPoint = "DownSample";
	DownSampleRCS.iWidth = m_iWinSizeX >> 1;
	DownSampleRCS.iHeight = m_iWinSizeY >> 1;
	DownSampleRCS.fDefinitionX = 16.f;
	DownSampleRCS.fDefinitionY = 16.f;
	DownSampleRCS.eFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	DownSampleRCS.iMipLevels = 3;
	DownSampleRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOWNSAMPLE"), &DownSampleRCS)))
		CRASH("Failed Add RCS_DOWNSAMPLE");

	//DOF Depth
	DownSampleRCS.iMipLevels = 2;
	DownSampleRCS.eFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_DOWNSAMPLE_DEPTH"), &DownSampleRCS)))
		CRASH("Failed Add RCS_DOWNSAMPLE_DEPTH");
#pragma endregion

#pragma region UPSAMPLE
	CRendererCS::RCS_DESC UpSampleRCS = {};
	UpSampleRCS.pFilePath = TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Sampling.hlsl");
	UpSampleRCS.eShaderMacro = { {"THREAD_X", "16" } ,{"THREAD_Y", "16" } ,{"THREAD_Z", "1" } , { NULL, NULL } };
	UpSampleRCS.strEntryPoint = "UpSample";
	UpSampleRCS.iWidth = m_iWinSizeX;
	UpSampleRCS.iHeight = m_iWinSizeY;
	UpSampleRCS.fDefinitionX = 16.f;
	UpSampleRCS.fDefinitionY = 16.f;
	UpSampleRCS.eFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	UpSampleRCS.iMipLevels = 3;
	UpSampleRCS.vClearColor = _float4(0.f, 0.f, 0.f, 0.f);

	//Default
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE_DEPTH");

	//UpSample Bloom
	UpSampleRCS.strEntryPoint = "UpSample_Bloom";
	if (FAILED(m_pGameInstance->Add_RCS(TEXT("RCS_UPSAMPLE_BLOOM"), &UpSampleRCS)))
		CRASH("Failed Add RCS_UPSAMPLE");
#pragma endregion

	return S_OK;
}

void CSFX_Hub::Update_Toggle(_float fTimeDelta)
{
	if (m_fToggleDuration == 0.f)
		return;

	m_fCurrentToggleDuration += fTimeDelta;
	if (m_fCurrentToggleDuration >= m_fToggleDuration)
		End_SFX();
}

void CSFX_Hub::Update_ToggleIntensity(_float fTimeDleta)
{
	_float fFluctuate = fTimeDleta * m_fIntensityBoost;

	if (false == m_IsToggleOn)
	{
		fFluctuate *= -1.f;
		if (m_fToggleIntensity <= 0.f)
		{
			m_pCurrentSFX->Exit();
			Safe_Release(m_pCurrentSFX);
			m_pCurrentSFX = nullptr;
			m_eCurrentToggle = SFX_TOGGLE::END;
		}
	}

	m_fToggleIntensity = clamp(m_fToggleIntensity + fFluctuate, 0.f, 1.f);
}

CSFX_Hub* CSFX_Hub::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CSFX_Hub* pInstance = new CSFX_Hub(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CSFX_Hub");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSFX_Hub::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	
	Safe_Release(m_pCurrentSFX);

	for (auto& Pair : m_SFXs)
		Safe_Release(Pair.second);
	m_SFXs.clear();

}

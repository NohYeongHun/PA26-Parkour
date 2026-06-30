#include "EnginePch.h"
#include "Water.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "GameInstance.h"

CWater::CWater(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSFX { pDevice, pContext }
{
}

HRESULT CWater::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_fMinStepSize = 10.f;
	m_fMaxStepSize = 20.f;
	m_fStartOffset = 10.f;

	m_fMaxDepth = 3.f;

	m_fMinTickness = 0.1f;
	m_fMaxTickness = 0.5f;

	return S_OK;
}

void CWater::Update(_float fTimeDelta)
{
}

HRESULT CWater::Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	if (FAILED(pShader->Bind_Texture("g_BackBufferTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Render Fail");

	if (FAILED(pShader->Bind_Value("g_fMinStepSize", &m_fMinStepSize, sizeof(_float))))
		CRASH("Failed Bind g_fMinStepSize");
	if (FAILED(pShader->Bind_Value("g_fMaxStepSize", &m_fMaxStepSize, sizeof(_float))))
		CRASH("Failed Bind g_fMaxStepSize");
	if (FAILED(pShader->Bind_Value("g_fStartOffset", &m_fStartOffset, sizeof(_float))))
		CRASH("Failed Bind g_fStartOffset");
	if (FAILED(pShader->Bind_Value("g_fMaxDepth", &m_fMaxDepth, sizeof(_float))))
		CRASH("Failed Bind g_fStartOffset");
	if (FAILED(pShader->Bind_Value("g_fMinTickness", &m_fMinTickness, sizeof(_float))))
		CRASH("Failed Bind g_fStartOffset");
	if (FAILED(pShader->Bind_Value("g_fMaxTickness", &m_fMaxTickness, sizeof(_float))))
		CRASH("Failed Bind g_fStartOffset");

	/*if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Diffuse"), pShader, "g_DiffuseTexture")))
		CRASH("Failed to Bind DiffuseTexture");*/

	if (FAILED(m_pGameInstance->Bind_EnvMapDatas(pShader, "g_EnvMapTexture", "g_EnvMapDatas", "g_HasEnvMap", "g_iNumEnvMaps")))
		CRASH("Failed Bind EnvMapDatas");

	if (FAILED(pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::WATER))))
		CRASH("Render Fail")

	pVIBuffer->Bind_Resources();
	pVIBuffer->Render();

	return S_OK;
}

CWater* CWater::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWater* pInstance = new CWater(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CWater");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWater::Free()
{
	__super::Free();
}

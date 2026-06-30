#include "EditorPch.h"
#include "AugustaBlur.h"

CAugustaBlur::CAugustaBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect { pDevice, pContext }
{
}

CAugustaBlur::CAugustaBlur(const CAugustaBlur& Prototype)
	: CEdit_ScreenEffect { Prototype }
{
}

HRESULT CAugustaBlur::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	if (FAILED(Ready_Buffer()))
		return E_FAIL;

	/*m_SlashData.vSlashPoint0 = _float2(1920.f, 884.f);
	m_SlashData.vSlashPoint1 = _float2(0.f, 784.f);
	m_SlashData.fOffset = 100.f;
	m_SlashData.fIntensity = 1.f;*/

	m_RadialData.fMinDistance = 0.f;
	m_RadialData.fMaxDistance = 0.65f;
	m_RadialData.fLengthScale = -0.4f;
	m_RadialData.vPivot = _float2(0.5f, 0.4f);

	m_vEffectTime = _float2(0.f, 0.5f);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	//memcpy(SubResource.pData, &m_SlashData, sizeof(SLASH_DATA));
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Noise_12001.png"), 1);
	ASSERT_CRASH(m_pNoiseTexture);

    return S_OK;
}

HRESULT CAugustaBlur::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

    return S_OK;
}

void CAugustaBlur::Priority_Update(_float fTimeDelta)
{
}

void CAugustaBlur::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	_float fRatio = 1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime);

//	m_SlashData.fIntensity = 1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime);
	m_RadialData.fLengthScale = (- 0.4f * fRatio);
}

void CAugustaBlur::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::POST_SFX, this)))
		return;
}

void CAugustaBlur::Render()
{
	ImGui::Begin("BAT");

	ImGui::InputFloat("Min", &m_RadialData.fMinDistance, 0.01f, 0.1f);
	ImGui::InputFloat("Max", &m_RadialData.fMaxDistance, 0.01f, 0.1f);

	ImGui::InputFloat("Length", &m_RadialData.fLengthScale, 0.01f, 0.1f);
	ImGui::InputFloat("PivotX", &m_RadialData.vPivot.x, 0.01f, 0.1f);
	ImGui::InputFloat("PivotY", &m_RadialData.vPivot.y, 0.01f, 0.1f);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	ImGui::End();

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vScreenSize", &m_vWinSize, sizeof(_float2))))
		CRASH("Failed to Bind vScreenSize");

	if(FAILED(m_pNoiseTexture->Bind_Shader_Resource(m_pShader, "g_NoiseTexture")))
		CRASH("Failed to Bind NoseTexture");
	
	if (FAILED(m_pShader->Bind_Texture("g_SceneTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed to Bind SceneTexture");

	m_pShader->Begin(1);

	m_pContext->PSSetConstantBuffers(0, 1, &m_pBuffer);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CAugustaBlur::Play()
{
	m_fCurrentTime = 0.f;
}

void CAugustaBlur::Stop()
{
}

void CAugustaBlur::Reset()
{
}

HRESULT CAugustaBlur::Ready_Buffer()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	//BufferDesc.ByteWidth = sizeof(SLASH_DATA);
	BufferDesc.ByteWidth = sizeof(RADIAL_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Create : Constant Buffer");


	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

CAugustaBlur* CAugustaBlur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAugustaBlur* pInstance = new CAugustaBlur(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAugustaBlur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAugustaBlur::Clone(void* pArg)
{
	CAugustaBlur* pInstance = new CAugustaBlur(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAugustaBlur");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CAugustaBlur::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
	Safe_Release(m_pBuffer);
	Safe_Release(m_pNoiseTexture);
}

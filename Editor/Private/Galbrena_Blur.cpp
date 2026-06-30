#include "EditorPch.h"
#include "Galbrena_Blur.h"

CGalbrena_Blur::CGalbrena_Blur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect{ pDevice, pContext }
{
}

CGalbrena_Blur::CGalbrena_Blur(const CGalbrena_Blur& Prototype)
	: CEdit_ScreenEffect{ Prototype }
{
}

HRESULT CGalbrena_Blur::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_vMaxDistance = _float2(1.f, 0.2f);
	m_vLengthScale = _float2(0.f, -0.4f);

	m_vReversTime[0] = _float2(0.1f, 0.15f);
	m_vReversTime[1] = _float2(0.4f, 0.5f);

	m_vRadialTime = _float2(0.4f, 0.6f);

	m_RadialData.fMinDistance = 0.f;
	m_RadialData.fMaxDistance = m_vMaxDistance.x;
	m_RadialData.fLengthScale = m_vLengthScale.x;
	m_RadialData.vPivot = _float2(0.7f, 0.75f);

	m_vEffectTime = _float2(0.f, 0.6f);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	m_pNoiseTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Noise_12001.png"), 1);
	ASSERT_CRASH(m_pNoiseTexture);

	return S_OK;
}

HRESULT CGalbrena_Blur::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CGalbrena_Blur::Priority_Update(_float fTimeDelta)
{
}

void CGalbrena_Blur::Update(_float fTimeDelta)
{
	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_pGameInstance->Setting_LUT(0, 0.25f, true);
		m_RadialData.fLengthScale = 0.f;
		D3D11_MAPPED_SUBRESOURCE SubResource = {};
		m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
		memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
		m_pContext->Unmap(m_pBuffer, 0);

		return;
	}

	m_fCurrentTime += fTimeDelta;

	_float fLUTRatio = SmoothStep(m_vEffectTime.x, m_vRadialTime.y, m_fCurrentTime);
	_float fRadialRatio = SmoothStep(m_vRadialTime.x, m_vRadialTime.y, m_fCurrentTime);
	
	m_RadialData.fMaxDistance = lerp(m_vMaxDistance.x, m_vMaxDistance.y, fRadialRatio);
	m_RadialData.fLengthScale = lerp(m_vLengthScale.x, m_vLengthScale.y, fRadialRatio);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, &m_RadialData, sizeof(RADIAL_DATA));
	m_pContext->Unmap(m_pBuffer, 0);

	m_IsRevers = false;

	for (_uint i = 0; i < 2; ++i)
	{
		if (m_fCurrentTime >= m_vReversTime[i].x && m_fCurrentTime <= m_vReversTime[i].y)
		{
			m_IsRevers = true;
			break;
		}

	}

	m_pGameInstance->Setting_LUT(3, fLUTRatio, true);
}

void CGalbrena_Blur::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::POST_SFX, this)))
		return;
}

void CGalbrena_Blur::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vScreenSize", &m_vWinSize, sizeof(_float2))))
		CRASH("Failed to Bind vScreenSize");

	if (FAILED(m_pShader->Bind_Value("g_IsRevers", &m_IsRevers, sizeof(_bool))))
		CRASH("Failed to Bind vScreenSize");

	if (FAILED(m_pNoiseTexture->Bind_Shader_Resource(m_pShader, "g_NoiseTexture")))
		CRASH("Failed to Bind NoseTexture");

	if (FAILED(m_pShader->Bind_Texture("g_SceneTexture", m_pGameInstance->Get_CurrentSceneSRV())))
		CRASH("Failed to Bind SceneTexture");

	m_pShader->Begin(3);

	m_pContext->PSSetConstantBuffers(0, 1, &m_pBuffer);

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrena_Blur::Play()
{
	m_fCurrentTime = 0.f;
}

void CGalbrena_Blur::Stop()
{
}

void CGalbrena_Blur::Reset()
{
}

HRESULT CGalbrena_Blur::Ready_Components()
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(RADIAL_DATA);
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer)))
		CRASH("Failed to Create : Constant Buffer");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

CGalbrena_Blur* CGalbrena_Blur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrena_Blur* pInstance = new CGalbrena_Blur(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrena_Blur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrena_Blur::Clone(void* pArg)
{
	CGalbrena_Blur* pInstance = new CGalbrena_Blur(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrena_Blur");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrena_Blur::Free()
{
	__super::Free();


	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
	Safe_Release(m_pBuffer);
	Safe_Release(m_pNoiseTexture);
}

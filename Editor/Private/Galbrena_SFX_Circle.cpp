#include "EditorPch.h"
#include "Galbrena_SFX_Circle.h"

CGalbrena_SFX_Circle::CGalbrena_SFX_Circle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect{ pDevice, pContext }
{
}

CGalbrena_SFX_Circle::CGalbrena_SFX_Circle(const CGalbrena_SFX_Circle& Prototype)
	: CEdit_ScreenEffect{ Prototype }
	, m_vCenter{ Prototype.m_vCenter }
	, m_vEffectTime{ Prototype.m_vEffectTime }
	, m_vScale{ Prototype.m_vScale }
	, m_vColor{ Prototype.m_vColor }
	, m_vSize{ Prototype.m_vSize }
{
}

HRESULT CGalbrena_SFX_Circle::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vCenter = _float2(m_vWinSize.x * 0.7f, m_vWinSize.y * 0.75f);

	m_vSize = _float2(300.f, 300.f);

	m_fRadius = 250.f;
	m_fCircleWidth = 30.f;

	m_vEffectTime = _float2(0.f, 0.5f);
	m_vScale = _float2(1.f, 4.f);

	m_vColor = _float3(1.f, 1.f, 1.f);

	m_fCurrentScale = m_vScale.x;
	m_fCurrentRadius = m_fRadius;
	m_vCurrentSize = m_vSize;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGalbrena_SFX_Circle::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CGalbrena_SFX_Circle::Priority_Update(_float fTimeDelta)
{
}

void CGalbrena_SFX_Circle::Update(_float fTimeDelta)
{
	if (false == m_IsPlay)
		return;

	m_fCurrentTime += fTimeDelta;

	if (m_fCurrentTime >= m_vEffectTime.y)
	{
		m_isActivate = false;
		return;
	}

	_float fRatio = SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime);

	m_fCurrentScale = lerp(m_vScale.x, m_vScale.y, fRatio);
}

void CGalbrena_SFX_Circle::Late_Update(_float fTimeDelta)
{
	ImGui::Begin("STAR");

	ImGui::InputFloat("SIZE_X", &m_vSize.x);
	ImGui::InputFloat("SIZE_Y", &m_vSize.y);

	ImGui::InputFloat("SCALE_X", &m_vScale.x);
	ImGui::InputFloat("SCALE_Y", &m_vScale.y);

	ImGui::InputFloat("RADIUS", &m_fRadius);
	ImGui::InputFloat("WIDTH", &m_fCircleWidth);

	ImGui::End();

	m_fCurrentRadius = m_fRadius * m_fCurrentScale;
	m_vCurrentSize = _float2(m_vSize.x * m_fCurrentScale, m_vSize.y * m_fCurrentScale);
	Setting_Scale(m_vCurrentSize.x, m_vCurrentSize.y);
	Setting_Pos(m_vCenter.x, m_vCenter.y);

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CGalbrena_SFX_Circle::Render()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fCircleRadius", &m_fCurrentRadius, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fCircleWidth", &m_fCircleWidth, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_vScreenSize", &m_vCurrentSize, sizeof(_float2))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(2);

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrena_SFX_Circle::Play()
{
	m_IsPlay = true;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;
	m_fCurrentRadius = m_fRadius;
	m_vCurrentSize = m_vSize;
}

void CGalbrena_SFX_Circle::Stop()
{
	m_IsPlay = false;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;
	m_fCurrentRadius = m_fRadius;
	m_vCurrentSize = m_vSize;
}

void CGalbrena_SFX_Circle::Reset()
{
}

HRESULT CGalbrena_SFX_Circle::Ready_Components()
{

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	m_pTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Ring_50002.png"), 1);
	ASSERT_CRASH(m_pTexture);

	return S_OK;
}

CGalbrena_SFX_Circle* CGalbrena_SFX_Circle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrena_SFX_Circle* pInstance = new CGalbrena_SFX_Circle(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrena_SFX_Circle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrena_SFX_Circle::Clone(void* pArg)
{
	CGalbrena_SFX_Circle* pInstance = new CGalbrena_SFX_Circle(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrena_SFX_Circle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrena_SFX_Circle::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
	Safe_Release(m_pTexture);
}

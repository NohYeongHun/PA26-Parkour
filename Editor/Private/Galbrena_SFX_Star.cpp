#include "EditorPch.h"
#include "Galbrena_SFX_Star.h"

CGalbrena_SFX_Star::CGalbrena_SFX_Star(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect{ pDevice, pContext }
{
}

CGalbrena_SFX_Star::CGalbrena_SFX_Star(const CGalbrena_SFX_Star& Prototype)
	: CEdit_ScreenEffect{ Prototype }
	, m_vCenter{ Prototype.m_vCenter }
	, m_vEffectTime{ Prototype.m_vEffectTime }
	, m_vScale{ Prototype.m_vScale }
	, m_vColor{ Prototype.m_vColor }
	, m_vSize{ Prototype.m_vSize }
{
	for (_uint i = 0; i < 2; ++i)
		m_fRotate[i] = Prototype.m_fRotate[i];
}

HRESULT CGalbrena_SFX_Star::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vCenter = _float2(m_vWinSize.x * 0.7f, m_vWinSize.y * 0.75f);

	m_vSize = _float2(m_vWinSize.x * 0.3f, m_vWinSize.y * 0.3f);

	m_vEffectTime = _float2(0.f, 0.5f);
	m_vScale = _float2(1.f, 2.f);

	m_fRotate[0] = 0.f;
	m_fRotate[1] = XMConvertToRadians(90.f);

	m_vColor = _float3(1.f, 1.f, 1.f);

	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Update_Instance();

	return S_OK;
}

HRESULT CGalbrena_SFX_Star::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CGalbrena_SFX_Star::Priority_Update(_float fTimeDelta)
{
}

void CGalbrena_SFX_Star::Update(_float fTimeDelta)
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

void CGalbrena_SFX_Star::Late_Update(_float fTimeDelta)
{
	//ImGui::Begin("STAR");

	//ImGui::InputFloat("SIZE_X", &m_vSize.x);
	//ImGui::InputFloat("SIZE_Y", &m_vSize.y);

	//ImGui::InputFloat("SCALE_X", &m_vScale.x);
	//ImGui::InputFloat("SCALE_Y", &m_vScale.y);

	//ImGui::End();

	Update_Instance();

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CGalbrena_SFX_Star::Render()
{
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind MaskTexture");

	m_pShader->Begin(1);

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrena_SFX_Star::Play()
{
	m_IsPlay = true;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;

	m_pVIBuffer->Clear();
}

void CGalbrena_SFX_Star::Stop()
{
	m_IsPlay = false;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = m_vScale.x;
}

void CGalbrena_SFX_Star::Reset()
{
}

HRESULT CGalbrena_SFX_Star::Ready_Components()
{

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect_Instance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	m_pTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_11000_WP20002.png"), 1);
	ASSERT_CRASH(m_pTexture);

	return S_OK;
}

void CGalbrena_SFX_Star::Update_Instance()
{
	vector<VTXINSTANCE_RECT> Datas;

	for (_uint i = 0; i < 2; ++i)
		Datas.push_back(Make_Instance(m_vCenter, m_vSize.x, m_vSize.y * m_fCurrentScale, m_fRotate[i]));

	m_pVIBuffer->Update_Buffer(Datas);
}

VTXINSTANCE_RECT CGalbrena_SFX_Star::Make_Instance(_float2 vCenter, _float fSizeX, _float fSizeY, _float fRotateZ)
{
	VTXINSTANCE_RECT Rect = {};

	_vector vRotation = XMQuaternionRotationRollPitchYaw(0.f, 0.f, fRotateZ);		// Rotate

	Setting_Scale(fSizeX, fSizeY);
	m_pTransformCom->Rotation_Quaternion(vRotation);
	Setting_Pos(vCenter.x, vCenter.y);

	memcpy(&Rect.vRight, m_pTransformCom->Get_WorldMatrixPtr(), sizeof(_matrix));

	return Rect;
}

CGalbrena_SFX_Star* CGalbrena_SFX_Star::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrena_SFX_Star* pInstance = new CGalbrena_SFX_Star(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrena_SFX_Star");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrena_SFX_Star::Clone(void* pArg)
{
	CGalbrena_SFX_Star* pInstance = new CGalbrena_SFX_Star(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrena_SFX_Star");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrena_SFX_Star::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
	Safe_Release(m_pTexture);
}

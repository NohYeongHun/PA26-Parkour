#include "EditorPch.h"
#include "Augusta_SFX.h"

CAugusta_SFX::CAugusta_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect { pDevice, pContext }
{
}

CAugusta_SFX::CAugusta_SFX(const CAugusta_SFX& Prototype)
	: CEdit_ScreenEffect { Prototype }
{
}

HRESULT CAugusta_SFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	if (FAILED(Ready_Textures()))
		return E_FAIL;

	m_vScale = _float2(m_vWinSize.x + (m_vWinSize.x * 2.f), m_vWinSize.y * 2.f);
	m_vPos = _float2(m_vWinSize.x * 0.5f, m_vWinSize.y * 0.4f);

	m_vEffectTime = _float2(0.f, 0.5f);
	
	Setting_Scale((m_vWinSize.x * 2.f), m_vWinSize.y * 0.5f);
	m_pTransformCom->Rotation_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, 0.f, XMConvertToRadians(20.f)));
	Setting_Pos(m_vPos.x, m_vPos.y);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 150.f, 0.f, 1.f));

	m_pGameInstance->Setting_Radial(_float2(0.5f, 0.3f), _float2(0.f, 0.0f), -1.5f);

	return S_OK;
}

HRESULT CAugusta_SFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CAugusta_SFX::Priority_Update(_float fTimeDelta)
{

}

void CAugusta_SFX::Update(_float fTimeDelta)
{
	m_fCurrentTime += fTimeDelta;

	Setting_Scale((m_vWinSize.x * 2.f), (m_vWinSize.y * 2.f) * (1.f - SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime)));
//	m_pTransformCom->Rotation_Quaternion(XMQuaternionRotationRollPitchYaw(0.f, 0.f, XMConvertToRadians(20.f)));
//	Setting_Pos(m_vPos.x, m_vPos.y);
}

void CAugusta_SFX::Late_Update(_float fTimeDelta)
{
	 
	if (m_pGameInstance->Get_DIKeyState(DIK_F10) == KEYSTATE::DOWN)
	{
		if (FAILED(m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL)))
			return;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_F11) == KEYSTATE::DOWN)
	{
		if (FAILED(m_pGameInstance->End_SFX()))
			return;
	}

	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CAugusta_SFX::Render()
{

	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if (FAILED(m_pMaskTexture->Bind_Shader_Resource(m_pShader, "g_MaskTexture")))
		CRASH("Failed to Bind MaskTexture");

	if(FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(0);

	m_pVIBuffer_Rect->Bind_Resources();
	m_pVIBuffer_Rect->Render();
}

void CAugusta_SFX::Play()
{
	m_fCurrentTime = 0.f;
	Setting_Scale((m_vWinSize.x * 2.f), m_vWinSize.y * 2.f);
}

void CAugusta_SFX::Stop()
{
}

void CAugusta_SFX::Reset()
{
}

HRESULT CAugusta_SFX::Ready_Textures()
{
//	m_pMaskTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_300156.png"), 1);
//	ASSERT_CRASH(m_pMaskTexture);

	m_pMaskTexture = CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/SFX/T_Mask_300156.png"), 1);
	ASSERT_CRASH(m_pMaskTexture);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer_Rect), nullptr)))
		ASSERT_CRASH(m_pVIBuffer_Rect);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	return S_OK;
}

CAugusta_SFX* CAugusta_SFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAugusta_SFX* pInstance = new CAugusta_SFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CAugusta_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAugusta_SFX::Clone(void* pArg)
{
	CAugusta_SFX* pInstance = new CAugusta_SFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAugusta_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAugusta_SFX::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer_Rect);
	Safe_Release(m_pShader);
	Safe_Release(m_pMaskTexture);
	Safe_Release(m_pNoiseTexture);
}

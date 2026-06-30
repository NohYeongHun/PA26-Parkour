#include "EditorPch.h"
#include "Edit_ScreenEffect.h"

CEdit_ScreenEffect::CEdit_ScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CEdit_ScreenEffect::CEdit_ScreenEffect(const CEdit_ScreenEffect& Prototype)
	: CGameObject { Prototype }
	, m_ViewMatrix { Prototype.m_ViewMatrix }
	, m_ProjMatrix { Prototype.m_ProjMatrix }
	, m_vWinSize {Prototype.m_vWinSize }
{

}

HRESULT CEdit_ScreenEffect::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.0f, 1.f));

	CGameObject::GAMEOBJECT_DESC Desc = {};
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;

	if (FAILED(__super::Initialize_Clone(&Desc)))
		return E_FAIL;

	m_isActivate = false;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_vWinSize = _float2(static_cast<_float>( g_iWinSizeX ), static_cast<_float>( g_iWinSizeY ));

	m_pTransformCom->Scale(_float3(m_vWinSize.x, m_vWinSize.y, 1.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(0.f, 0.f, 0.f, 1.f));

	return S_OK;
}

HRESULT CEdit_ScreenEffect::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_isActivate = false;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Scale(_float3(g_iWinSizeX, g_iWinSizeY, 1.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f, 1.f));

    return S_OK;
}

void CEdit_ScreenEffect::Priority_Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Late_Update(_float fTimeDelta)
{
}

void CEdit_ScreenEffect::Render()
{
}

HRESULT CEdit_ScreenEffect::Ready_Components()
{
	return S_OK;
}

void CEdit_ScreenEffect::Setting_Scale(_float fSizeX, _float fSizeY)
{
	if(fSizeY > 0.f)
		m_pTransformCom->Scale(_float3(fSizeX, fSizeY, 1.f));
}

void CEdit_ScreenEffect::Setting_Pos(_float fPosX, _float fPosY)
{
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(fPosX - (g_iWinSizeX * 0.5f), -fPosY + g_iWinSizeY * 0.5f, 0.f, 1.f));
}

void CEdit_ScreenEffect::Free()
{
	__super::Free();
}

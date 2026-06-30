#include "EnginePch.h"
#include "UIObject.h"

#include "Transform.h"

CUIObject::CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CUIObject::CUIObject(const CUIObject& Prototype)
	: CGameObject{ Prototype }
{
}

HRESULT CUIObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIObject::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	UI_DESC* pDesc = static_cast<UI_DESC*>(pArg);

	m_fX = pDesc->fX;
	m_fY = pDesc->fY;
	m_fSizeX = pDesc->fSizeX;
	m_fSizeY = pDesc->fSizeY;

	D3D11_VIEWPORT			Viewport{};
	_uint			iNumViewports = { 1 };

	m_pContext->RSGetViewports(&iNumViewports, &Viewport);
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(Viewport.Width, Viewport.Height, 0.0f, 1.f));

	m_iWinSizeX = Viewport.Width;
	m_iWinSizeY = Viewport.Height;

	return S_OK;
}

void CUIObject::Priority_Update(_float fTimeDelta)
{
}

void CUIObject::Update(_float fTimeDelta)
{
}

void CUIObject::Late_Update(_float fTimeDelta)
{
}

void CUIObject::Render()
{
	return;
}

_bool CUIObject::Check_OnInteract(_uint iEventInteractType, _uint iInstanceIndex)
{
	return _bool();
}

void CUIObject::OnEvent(_uint iEventType)
{
}

void CUIObject::Update_InputState()
{
}

_bool CUIObject::Check_IsInSpace()
{
	return _bool();
}

HRESULT CUIObject::Begin()
{
	m_pTransformCom->Scale(_float3(m_fSizeX, m_fSizeY, 1.f));
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(m_fX - m_iWinSizeX * 0.5f, -m_fY + m_iWinSizeY * 0.5f, 0.f, 1.f));


	return S_OK;
}

void CUIObject::Free()
{
	__super::Free();
}

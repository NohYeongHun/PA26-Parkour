#include "ClientPch.h"
#include "Mouse.h"

#include "GameSystem.h"

CMouse::CMouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext },
	m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

CMouse::CMouse(const CMouse& Prototype)
	: CGameObject { Prototype },
	m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMouse::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMouse::Initialize_Clone(void* pArg)
{
	m_iCursorPosX = g_iWinSizeX >> 1;
	m_iCursorPosY = g_iWinSizeY >> 1;

	Ready_Component();

	m_pGameSystem->Register_Mouse(this);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	ShowCursor(false);

    return S_OK;
}

void CMouse::Priority_Update(_float fTimeDelta)
{
}

void CMouse::Update(_float fTimeDelta)
{
	// Debug
	if (m_pGameInstance->Get_DIKeyState(DIK_M) == KEYSTATE::DOWN)
		m_isMouseFix = !m_isMouseFix;

	if (true == m_isMouseFix)
	{
		m_iCursorPosX = g_iWinSizeX >> 1;
		m_iCursorPosY = g_iWinSizeY >> 1;
		SetCursorPos(m_iCursorPosX, m_iCursorPosY);
	}
	else
	{
		POINT ptMouse = {};
		GetCursorPos(&ptMouse);
		ScreenToClient(g_hWnd, &ptMouse);
		m_iCursorPosX = ptMouse.x;
		m_iCursorPosY = ptMouse.y;
	}
	Compute_XY();
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(50.f, 50.f, 1.f) * XMMatrixTranslation(m_iCursorPosX, m_iCursorPosY, 0.f));
}

void CMouse::Late_Update(_float fTimeDelta)
{
	if (true == m_isMouseFix)
		return;

	m_pGameInstance->Add_Render_Object(RENDERGROUP::UI_POST, this);
}

void CMouse::Render()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		CRASH("World");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("View");
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Proj");

	if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture")))
		CRASH("Texture");

	m_pShaderCom->Begin(1);
	m_pVIBufferCom->Bind_Resources();
	m_pVIBufferCom->Render();
}

void CMouse::Ready_Component()
{
	// Com_Texture
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Mouse"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
		CRASH("Texture");

	// Com_VIBuffer
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		CRASH("VIBuffer");

	// Com_Shader
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_Mouse"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");
}

void CMouse::Compute_XY()
{
	m_iCursorPosX = m_iCursorPosX - (g_iWinSizeX >> 1);
	m_iCursorPosY = -m_iCursorPosY + (g_iWinSizeY >> 1);
}

CMouse* CMouse::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMouse* pInstance = new CMouse(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Mouse");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMouse::Clone(void* pArg)
{
	CMouse* pClone = new CMouse(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mouse");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMouse::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pGameSystem);
}

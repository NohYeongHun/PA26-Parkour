#include "EnginePch.h"
#include "CustomFont.h"

#include "GameInstance.h"

CCustomFont::CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject(pDevice, pContext)
{
}

CCustomFont::CCustomFont(const CCustomFont& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CCustomFont::Initialize_Prototype()
{
	__super::Initialize_Prototype();

	return S_OK;
}

HRESULT CCustomFont::Initialize_Clone(void* pArg)
{
	__super::Initialize_Clone(pArg);

	FONT_SINGLEDESC* pDesc = static_cast<FONT_SINGLEDESC*>(pArg);

	m_tSingleDesc	= *pDesc;

	return S_OK;
}

void CCustomFont::Priority_Update(_float fTimeDelta)
{
}

void CCustomFont::Update(_float fTimeDelta)
{
	// Update Lifetime. If Lifetimes end, disable activate.

	if (m_tSingleDesc.vLifeTime.y <= m_tSingleDesc.vLifeTime.x)
		m_isActivate = false;

	m_tSingleDesc.vLifeTime.x += fTimeDelta;
}

void CCustomFont::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::UI, this)))
		return;
}

void CCustomFont::Render()
{
	//m_pGameInstance->Draw_Font(&m_tSingleDesc);
}

CCustomFont* CCustomFont::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCustomFont* pInstance = new CCustomFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("Failed to Created : CCustomFont");

	return pInstance;
}

CGameObject* CCustomFont::Clone(void* pArg)
{
	CCustomFont* pInstance = new CCustomFont(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
		CRASH("Failed to Created : CCustomFont (Clone)");

	return pInstance;
}

void CCustomFont::Free()
{
	__super::Free();
}


#pragma region old

//CCustomFont::CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//	: m_pDevice { pDevice }, m_pContext { pContext }
//{
//	Safe_AddRef(m_pDevice);
//	Safe_AddRef(m_pContext);
//}
//
//HRESULT CCustomFont::Initialize(const _tchar* pFilePath)
//{
//	m_pBatch = new SpriteBatch(m_pContext);
//	m_pFont = new SpriteFont(m_pDevice, pFilePath);
//
//	return S_OK;
//}
//
//HRESULT CCustomFont::Render(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale)
//{
//	m_pBatch->Begin();
//	m_pFont->DrawString(m_pBatch, pText, vPosition, vColor, fRadian, vOrigin, vScale);
//	m_pBatch->End();
//	return S_OK;
//}
//
//CCustomFont* CCustomFont::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath)
//{
//	CCustomFont* pInstance = new CCustomFont(pDevice, pContext);
//
//	if (FAILED(pInstance->Initialize(pFilePath)))
//	{
//		MSG_BOX("Failed to Create : Font");
//		Safe_Release(pInstance);
//	}
//
//	return pInstance;
//}
//
//void CCustomFont::Free()
//{
//	__super::Free();
//
//	delete m_pBatch;
//	delete m_pFont;
//
//	Safe_Release(m_pDevice);
//	Safe_Release(m_pContext);
//}

#pragma endregion

#include "EnginePch.h"

#include "UI_Manager.h"
#include "UIObject.h"

CUI_Manager::CUI_Manager()
{

}

HRESULT CUI_Manager::Initialize()
{
	return S_OK;
}

void CUI_Manager::Update(_float fTimeDelta)
{
	for (auto& rootUI : m_RootUIs)
	{
		//rootUI.second->Update();
	}

	// �ٵ� �׷��� ���⼭ �� ��������?

	// Update�� ������Ʈ�Ŵ������������µ�
}

HRESULT CUI_Manager::SetActive_UI(const _wstring& strName_UI, _bool isActive)
{
	CUIObject* pUIObject = Find_UIObject(strName_UI);
	if (!pUIObject)
		return E_FAIL;

	pUIObject->Set_Active(isActive);
	return S_OK;
}

 
// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Find_ChildObject(L"SectorB_Status")->Check_OnInteract(ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0);
// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Check_OnInteract(L"SectorB_Status", ENUM_CLASS(UI_EVENT_TYPE::CLICK_ENTER), 0);
//
//
// - �ܺο��� UI���� ���� ������
// dynamic_cast<CCustom_UI*>(m_pGameInstance->Find_UIObject(L"UI_UHD"))->Find_ChildObject(L"SectorB_Status")->OnEvent(UI_EVENT_TYPE::CLICK_ENTER);



CUIObject* CUI_Manager::Find_UIObject(const _wstring& strName_UI)
{
	auto iter = m_RootUIs.find(strName_UI);
	if (iter != m_RootUIs.end())
		return iter->second;

	return nullptr;
}

HRESULT CUI_Manager::Add_RootUI(const _wstring& strName_UI, CUIObject* pRootUI)
{
	if (pRootUI == nullptr)
		return E_FAIL;

	if (m_RootUIs.find(strName_UI) != m_RootUIs.end())
		return E_FAIL;

	m_RootUIs.emplace(strName_UI, pRootUI);
	return S_OK;
}

HRESULT CUI_Manager::Remove_RootUI(const _wstring& strName_UI)
{
	auto iter = m_RootUIs.find(strName_UI);

	if (iter == m_RootUIs.end())
		return E_FAIL;

	m_RootUIs.erase(iter);

	return S_OK;
}

void CUI_Manager::Clear_RootUI()
{
	m_RootUIs.clear();
}

CUI_Manager* CUI_Manager::Create()
{
	CUI_Manager* pInstance = new CUI_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : UI_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Manager::Free()
{
	__super::Free();
}

#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CUI_Manager final : public CBase
{
private:
	explicit			CUI_Manager();
	virtual				~CUI_Manager() = default;

public:
	HRESULT				Initialize();
	void				Update(_float fTimeDelta);

public:
	HRESULT				SetActive_UI(const _wstring& strName_UI, _bool isActive);
	//_bool				Check_UIEvent_Triggered(const _wstring& strName_UI, _uint iCheckEventType);
	class CUIObject*	Find_UIObject(const _wstring& strName_UI);

	HRESULT				Add_RootUI(const _wstring& strName_UI, class CUIObject* rootUI);
	HRESULT				Remove_RootUI(const _wstring& strName_UI);
	void				Clear_RootUI();

private:
	map<_wstring, class CUIObject*>		m_RootUIs = {};

public:
	static		CUI_Manager*	Create();
	virtual		void			Free() override;

};

NS_END	
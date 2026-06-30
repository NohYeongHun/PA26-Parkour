#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CObjectFuncNotify final : public CAnimNotify
{
public:
	explicit CObjectFuncNotify(_float fTrackPosition, const _string& strObjectTag);
	void Execute() override;
	json To_Json() const override;
	const _string& Get_NotifyTypeName() const override;
	
	void Set_ObjectCallback(function<void(const _wstring&)> ObjectCallback) { m_ObjectCallback = ObjectCallback; }

#ifdef _DEBUG
	void ImGui_Print() override;
#endif // _DEBUG

public:
	static CObjectFuncNotify* From_Json(const json& objectJson);
	virtual void Free() override;

private:
	function<void(const _wstring&)> m_ObjectCallback;

	_string m_strObjectTag = {};
	_wstring m_wStrObjectTag = {};
};
NS_END

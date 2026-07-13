#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CStateFlagNotify final : public CAnimNotify
{
public:
	explicit CStateFlagNotify(_float fTrackPosition, const _string& strFlagName, _bool isOn);
	void Execute() override;
	json To_Json() const override;
	const _string& Get_NotifyTypeName() const override;

	void Set_StateFlagCallback(function<void(const _string&, _bool)> Callback) { m_StateFlagCallback = Callback; }

#ifdef _DEBUG
	void ImGui_Print() override;
#endif // _DEBUG

public:
	static CStateFlagNotify* From_Json(const json& stateFlagJson);
	virtual void Free() override;

private:
	function<void(const _string&, _bool)> m_StateFlagCallback;

	_string m_strFlagName = {};
	_bool   m_isOn = true;   // 시작 지점 = true, 끝 지점 = false
};
NS_END

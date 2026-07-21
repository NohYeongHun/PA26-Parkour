#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CIKNotify final : public CAnimNotify
{
public:
	explicit CIKNotify(_float fTrackPosition);
	void Execute() override;
	json To_Json() const override;
	const _string& Get_NotifyTypeName() const override;

	void Set_IKCallback(function<void(const vector<IK_BINDING>& bindings, _float fBlendSec, _bool isBegin)> Callback) { m_IKCallBack = Callback; }

#ifdef _DEBUG
	void ImGui_Print() override;
#endif // _DEBUG

public:
	static EIKTARGET_MODE To_Mode(const string& s);
	static _string Mode_ToString(EIKTARGET_MODE eMode);
public:
	static CIKNotify* From_Json(const json& j, _bool isBegin);
	virtual void Free() override;

private:
	function<void(const vector<IK_BINDING>& bindings, _float fBlendSec, _bool isBegin)> m_IKCallBack;
	vector<IK_BINDING> m_Bindings{};
	_bool  m_isBegin = {};

	_float m_fEndTrackPosition = {};
	_float m_fBlendInSec = {};
	_float m_fBlendOutSec = {};
	

};
NS_END

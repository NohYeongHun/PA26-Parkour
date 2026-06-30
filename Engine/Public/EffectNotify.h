#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CEffectNotify final : public CAnimNotify
{
public:
	explicit CEffectNotify(_float fTrackPosition, const _string& strEffectTag);
	virtual void Execute() override;
	virtual json To_Json() const override;
	virtual const _string& Get_NotifyTypeName() const override;

#ifdef _DEBUG
	virtual void ImGui_Print() override;
#endif // _DEBUG

public:
	

public:
	static CEffectNotify* From_Json(const json& colliderJson);
	virtual void Free() override;

private:
	_string m_strEffectTag = {};
	_wstring m_wStrEffectTag = {};
};
NS_END

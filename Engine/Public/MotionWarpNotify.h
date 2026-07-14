#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CMotionWarpNotify final : public CAnimNotify
{
public:
	explicit CMotionWarpNotify(_float fTrackPosition, const _string& strTargetName,
	                           _bool isStart, _float fWindowEndTrackPos,
	                           _bool bWarpTranslation, _bool bWarpRotation);
	void Execute() override;
	json To_Json() const override;
	const _string& Get_NotifyTypeName() const override;

	void Set_WarpCallback(function<void(const _string&, _bool, _float, _bool, _bool)> cb)
	{ m_WarpCallback = cb; }

#ifdef _DEBUG
	void ImGui_Print() override;
#endif

public:
	static CMotionWarpNotify* From_Json(const json& warpJson);
	virtual void Free() override;

private:
	function<void(const _string& strTargetName, _bool isStart,
	              _float fWindowEndTrackPos, _bool bTrans, _bool bRot)> m_WarpCallback;
	_string m_strTargetName{};
	_bool   m_isStart = true;
	_float  m_fWindowEndTrackPos = 0.f;
	_bool   m_bWarpTranslation = true;
	_bool   m_bWarpRotation = false;
};
NS_END

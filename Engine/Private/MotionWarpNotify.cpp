#include "EnginePch.h"
#include "MotionWarpNotify.h"

CMotionWarpNotify::CMotionWarpNotify(_float fTrackPosition, const _string& strTargetName,
                                     _bool isStart, _float fWindowEndTrackPos,
                                     _bool isWarpTranslation, _bool isWarpRotation)
	: CAnimNotify(fTrackPosition)
	, m_strTargetName(strTargetName)
	, m_isStart(isStart)
	, m_fWindowEndTrackPos(fWindowEndTrackPos)
	, m_bWarpTranslation(isWarpTranslation)
	, m_bWarpRotation(isWarpRotation)
{
	m_strNotifyTypeName = "MotionWarp";
}

void CMotionWarpNotify::Execute()
{
	if (m_WarpCallback)
		m_WarpCallback(m_strTargetName, m_isStart, m_fWindowEndTrackPos, m_bWarpTranslation, m_bWarpRotation);
}

const _string& CMotionWarpNotify::Get_NotifyTypeName() const
{
	return m_strNotifyTypeName;
}

json CMotionWarpNotify::To_Json() const
{
	json j;
	j["NotifyType"]       = "MotionWarp";
	j["TrackPosition"]    = m_fTrackPosition;
	j["EndTrackPosition"] = m_fWindowEndTrackPos;
	j["TargetName"]       = m_strTargetName;
	j["WarpTranslation"]  = m_bWarpTranslation;
	j["WarpRotation"]     = m_bWarpRotation;
	return j;
}

CMotionWarpNotify* CMotionWarpNotify::From_Json(const json& warpJson)
{
	_float  fStart  = warpJson.value("TrackPosition", 0.f);
	_float  fEnd    = warpJson.value("EndTrackPosition", fStart);
	_string strName = warpJson.value("TargetName", string(""));
	_bool   bTrans  = warpJson.value("WarpTranslation", true);
	_bool   bRot    = warpJson.value("WarpRotation", false);
	// 시작점 노티파이 (isStart=true). 끝점은 팩토리(Load_Notify)에서 별도 생성한다.
	return new CMotionWarpNotify(fStart, strName, true, fEnd, bTrans, bRot);
}

#ifdef _DEBUG
void CMotionWarpNotify::ImGui_Print()
{
}
#endif

void CMotionWarpNotify::Free()
{
	__super::Free();
}

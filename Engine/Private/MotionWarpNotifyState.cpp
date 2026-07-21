#include "EnginePch.h"
#include "MotionWarpNotifyState.h"

CMotionWarpNotifyState::CMotionWarpNotifyState(_float fBeginTrackPos, _float fEndTrackPos,
                                               const _string& strTargetName,
                                               _bool isWarpTranslation, _bool isWarpRotation)
    : CAnimNotifyState{ fBeginTrackPos, fEndTrackPos }
    , m_strTargetName{ strTargetName }
    , m_bWarpTranslation{ isWarpTranslation }
    , m_bWarpRotation{ isWarpRotation }
{
    m_strNotifyTypeName = "MotionWarp";
}

void CMotionWarpNotifyState::On_NotifyBegin()
{
    if (m_WarpCallback)
        m_WarpCallback(m_strTargetName, true, m_fEndTrackPosition, m_bWarpTranslation, m_bWarpRotation);
}

void CMotionWarpNotifyState::On_NotifyEnd()
{
    if (m_WarpCallback)
        m_WarpCallback(m_strTargetName, false, m_fEndTrackPosition, m_bWarpTranslation, m_bWarpRotation);
}

json CMotionWarpNotifyState::To_Json() const
{
    json j;
    j["NotifyType"] = "MotionWarp";
    j["TrackPosition"] = m_fBeginTrackPosition;
    j["EndTrackPosition"] = m_fEndTrackPosition;
    j["TargetName"] = m_strTargetName;
    j["WarpTranslation"] = m_bWarpTranslation;
    j["WarpRotation"] = m_bWarpRotation;
    j["IsState"] = true;
    return j;
}

const _string& CMotionWarpNotifyState::Get_NotifyTypeName() const
{
    return m_strNotifyTypeName;
}

#ifdef _DEBUG
void CMotionWarpNotifyState::ImGui_Print()
{
    ImGui::Text("Window : %.2f ~ %.2f", m_fBeginTrackPosition, m_fEndTrackPosition);
    ImGui::Text("Target : %s", m_strTargetName.c_str());
    ImGui::Text("Active : %s", m_isActive ? "true" : "false");
}
#endif

CMotionWarpNotifyState* CMotionWarpNotifyState::From_Json(const json& j)
{
    ASSERT_CRASH(j.contains("EndTrackPosition"));

    const _float fBegin = j.value("TrackPosition", 0.f);
    const _float fEnd = j["EndTrackPosition"].get<_float>();
    ASSERT_CRASH(fEnd > fBegin);

    return new CMotionWarpNotifyState(fBegin, fEnd,
        j.value("TargetName", string("")),
        j.value("WarpTranslation", true),
        j.value("WarpRotation", false));
}

void CMotionWarpNotifyState::Free()
{
    CAnimNotifyState::Free();
}

#include "EnginePch.h"
#include "StateFlagNotifyState.h"

CStateFlagNotifyState::CStateFlagNotifyState(_float fBeginTrackPos, _float fEndTrackPos, const _string& strFlagName)
    : CAnimNotifyState{ fBeginTrackPos, fEndTrackPos }
    , m_strFlagName{ strFlagName }
{
    m_strNotifyTypeName = "StateFlag";
}

void CStateFlagNotifyState::On_NotifyBegin()
{
    if (m_StateFlagCallback)
        m_StateFlagCallback(m_strFlagName, true);
}

void CStateFlagNotifyState::On_NotifyEnd()
{
    if (m_StateFlagCallback)
        m_StateFlagCallback(m_strFlagName, false);
}

json CStateFlagNotifyState::To_Json() const
{
    json j;
    j["NotifyType"] = "StateFlag";
    j["TrackPosition"] = m_fBeginTrackPosition;
    j["EndTrackPosition"] = m_fEndTrackPosition;
    j["FlagName"] = m_strFlagName;
    j["IsState"] = true;
    return j;
}

const _string& CStateFlagNotifyState::Get_NotifyTypeName() const
{
    return m_strNotifyTypeName;
}

#ifdef _DEBUG
void CStateFlagNotifyState::ImGui_Print()
{
    ImGui::Text("Window : %.2f ~ %.2f", m_fBeginTrackPosition, m_fEndTrackPosition);
    ImGui::Text("Flag Name : %s", m_strFlagName.c_str());
    ImGui::Text("Active : %s", m_isActive ? "true" : "false");
}
#endif // _DEBUG

CStateFlagNotifyState* CStateFlagNotifyState::From_Json(const json& j)
{
    ASSERT_CRASH(j.contains("EndTrackPosition"));

    const _float fBegin = j["TrackPosition"].get<_float>();
    const _float fEnd = j["EndTrackPosition"].get<_float>();
    ASSERT_CRASH(fEnd > fBegin);  

    return new CStateFlagNotifyState(fBegin, fEnd, j["FlagName"].get<string>());
}

void CStateFlagNotifyState::Free()
{
    CAnimNotifyState::Free();
}

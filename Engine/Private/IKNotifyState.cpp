#include "EnginePch.h"
#include "IKNotifyState.h"

CIKNotifyState::CIKNotifyState(_float fBeginTrackPos, _float fEndTrackPos)
    : CAnimNotifyState{ fBeginTrackPos, fEndTrackPos }
{
    m_strNotifyTypeName = "IK";
}

void CIKNotifyState::On_NotifyBegin()
{
    if (m_IKCallBack)
        m_IKCallBack(m_Bindings, m_fBlendInSec, true);
}

void CIKNotifyState::On_NotifyEnd()
{
    if (m_IKCallBack)
        m_IKCallBack(m_Bindings, m_fBlendOutSec, false);
}

json CIKNotifyState::To_Json() const
{
    json j;
    j["NotifyType"] = "IK";
    j["TrackPosition"] = m_fBeginTrackPosition;
    j["EndTrackPosition"] = m_fEndTrackPosition;
    j["BlendInSec"] = m_fBlendInSec;
    j["BlendOutSec"] = m_fBlendOutSec;
    j["RampLen"] = m_fRampLen;
    j["IsState"] = true;

    json bindings = json::array();
    for (const auto& b : m_Bindings) {
        bindings.push_back({
            { "Goal",         b.strGoalName },
            { "Mode",         Mode_ToString(b.eMode) },
            { "TargetSource", b.strTargetSource },
            { "PosWeight",    b.fPosWeight },
            { "RotWeight",    b.fRotWeight },
			{ "IsFix",		  b.isFix},
            { "IsWallProbe",  b.isWallProbe },
            { "ProbeOut",     b.fProbeOut },
            { "ProbeDepth",   b.fProbeDepth },
            { "Skin",         b.fSkin },
            { "ReachRadius",  b.fReachRadius }
            });
    }
    j["Bindings"] = bindings;
    return j;
}

const _string& CIKNotifyState::Get_NotifyTypeName() const
{
    return m_strNotifyTypeName;
}

#ifdef _DEBUG
void CIKNotifyState::ImGui_Print()
{
    ImGui::Text("Window : %.2f ~ %.2f", m_fBeginTrackPosition, m_fEndTrackPosition);
    ImGui::Text("Bindings : %zu", m_Bindings.size());
    ImGui::Text("Active : %s", m_isActive ? "true" : "false");
}
#endif // _DEBUG

EIKTARGET_MODE CIKNotifyState::To_Mode(const string& s)
{
    if (s == "POSITION_CLEARANCE" || s == "POSITION_ROTATION")
        return EIKTARGET_MODE::POSITION_CLEARANCE;
    if (s == "POSITION")
        return EIKTARGET_MODE::POSITION;

    return EIKTARGET_MODE::END;
}

_string CIKNotifyState::Mode_ToString(EIKTARGET_MODE eMode)
{
    _string result = "END";

    if (eMode == EIKTARGET_MODE::POSITION_CLEARANCE)
        result = "POSITION_CLEARANCE";
    else if (eMode == EIKTARGET_MODE::POSITION)
        result = "POSITION";

    return result;
}

CIKNotifyState* CIKNotifyState::From_Json(const json& j)
{
    ASSERT_CRASH(j.contains("EndTrackPosition"));

    const _float fBegin = j.value("TrackPosition", 0.f);
    const _float fEnd = j["EndTrackPosition"].get<_float>();
    ASSERT_CRASH(fEnd > fBegin);

    CIKNotifyState* pInstance = new CIKNotifyState(fBegin, fEnd);
    pInstance->m_fBlendInSec = j.value("BlendInSec", 0.15f);
    pInstance->m_fBlendOutSec = j.value("BlendOutSec", 0.15f);
    pInstance->m_fRampLen = j.value("RampLen", 0.f);

    for (const auto& b : j.at("Bindings"))
    {
        IK_BINDING bind{};
        bind.strGoalName = b.value("Goal", string(""));
        bind.eMode = To_Mode(b.value("Mode", string("POSITION")));
        bind.strTargetSource = b.value("TargetSource", string(""));
        bind.fPosWeight = b.value("PosWeight", 1.f);
        bind.fRotWeight = b.value("RotWeight", 1.f);
		bind.isFix		= b.value("IsFix", false);
        bind.isWallProbe = b.value("IsWallProbe", false);
        bind.fProbeOut   = b.value("ProbeOut", 0.3f);
        bind.fProbeDepth = b.value("ProbeDepth", 0.6f);
        bind.fSkin       = b.value("Skin", 0.02f);
        bind.fReachRadius = b.value("ReachRadius", 0.f);
        pInstance->m_Bindings.push_back(bind);
    }
    return pInstance;
}

void CIKNotifyState::Free()
{
    CAnimNotifyState::Free();
}

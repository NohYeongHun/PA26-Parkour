#include "EnginePch.h"
#include "StateFlagNotify.h"

CStateFlagNotify::CStateFlagNotify(_float fTrackPosition, const _string& strFlagName, _bool isOn)
	: CAnimNotify{ fTrackPosition }
	, m_strFlagName{ strFlagName }
	, m_isOn{ isOn }
{
}

void CStateFlagNotify::Execute()
{
	if (m_StateFlagCallback)
		m_StateFlagCallback(m_strFlagName, m_isOn);
}

json CStateFlagNotify::To_Json() const
{
	json stateFlagJson;
	stateFlagJson["NotifyType"] = "StateFlag";
	stateFlagJson["TrackPosition"] = m_fTrackPosition;
	stateFlagJson["FlagName"] = m_strFlagName;
	stateFlagJson["IsOn"] = m_isOn;

	return stateFlagJson;
}

const _string& CStateFlagNotify::Get_NotifyTypeName() const
{
	static const _string typeName = "StateFlag";
	return typeName;
}

#ifdef _DEBUG
void CStateFlagNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
	ImGui::Text("Flag Name : %s", m_strFlagName.c_str());
	ImGui::Text("IsOn : %s", m_isOn ? "true" : "false");
}
#endif // _DEBUG

CStateFlagNotify* CStateFlagNotify::From_Json(const json& stateFlagJson)
{
	CStateFlagNotify* pInstance = new CStateFlagNotify(
		stateFlagJson["TrackPosition"],
		stateFlagJson["FlagName"],
		stateFlagJson.value("IsOn", true)   // 저장 왕복 시 점 엔트리 2개 형태 허용
	);

	return pInstance;
}

void CStateFlagNotify::Free()
{
	CAnimNotify::Free();
}

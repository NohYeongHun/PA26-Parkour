#include "EnginePch.h"
#include "IKNotify.h"

CIKNotify::CIKNotify(_float fTrackPosition)
	: CAnimNotify{ fTrackPosition }
{
}

void CIKNotify::Execute()
{
	if (m_IKCallBack)
		m_IKCallBack(m_Bindings, m_isBegin ? m_fBlendInSec : m_fBlendOutSec, m_isBegin);
}

json CIKNotify::To_Json() const
{
	if (!m_isBegin)                 // End 객체는 저장하지 않음 (Begin이 통째로 담음)
		return json::object();

	json j;
	j["NotifyType"] = "IK";
	j["TrackPosition"] = m_fTrackPosition;		
	j["EndTrackPosition"] = m_fEndTrackPosition;
	j["BlendInSec"] = m_fBlendInSec;
	j["BlendOutSec"] = m_fBlendOutSec;

	json bindings = json::array();
	for (const auto& b : m_Bindings) {
		bindings.push_back({
			{ "Goal",         b.strGoalName },
			{ "Mode",         Mode_ToString(b.eMode) },
			{ "TargetSource", b.strTargetSource },
			{ "PosWeight",    b.fPosWeight },
			{ "RotWeight",    b.fRotWeight }
			});
	}
	j["Bindings"] = bindings;
	return j;
}

const _string& CIKNotify::Get_NotifyTypeName() const
{
	static const _string typeName = "IK";
	return typeName;
}

#ifdef _DEBUG
void CIKNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
}
#endif // _DEBUG

EIKTARGET_MODE CIKNotify::To_Mode(const string& s)
{
	if (s == "POSITION_ROTATION")
		return EIKTARGET_MODE::POSITION_ROTATION;
	if (s == "POSITION")
		return EIKTARGET_MODE::POSITION;
	
	return EIKTARGET_MODE::END;
}

_string CIKNotify::Mode_ToString(EIKTARGET_MODE eMode)
{
	_string result = "END";

	if (eMode == EIKTARGET_MODE::POSITION_ROTATION)
		result = "POSITION_ROTATION";
	else if (eMode == EIKTARGET_MODE::POSITION)
		result = "POSITION";

	return result;
}

CIKNotify* CIKNotify::From_Json(const json& j, _bool isBegin)
{
	CIKNotify* pInstance = new CIKNotify(0.f);
	pInstance->m_isBegin = isBegin;
	pInstance->m_fTrackPosition = isBegin ? j.value("TrackPosition", 0.f)
		: j.value("EndTrackPosition", 0.f);
	pInstance->m_fEndTrackPosition = j.value("EndTrackPosition", 0.f);
	pInstance->m_fBlendInSec = j.value("BlendInSec", 0.15f);      
	pInstance->m_fBlendOutSec = j.value("BlendOutSec", 0.15f);    

	for (const auto& b : j.at("Bindings"))
	{
		IK_BINDING bind{};
		bind.strGoalName = b.value("Goal", string(""));
		bind.eMode = To_Mode(b.value("Mode", string("POSITION")));
		bind.strTargetSource = b.value("TargetSource", string(""));
		bind.fPosWeight = b.value("PosWeight", 1.f);
		bind.fRotWeight = b.value("RotWeight", 1.f);
		pInstance->m_Bindings.push_back(bind);
	}
	return pInstance;
}

void CIKNotify::Free()
{
	CAnimNotify::Free();
}

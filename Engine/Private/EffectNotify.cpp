#include "EnginePch.h"
#include "EffectNotify.h"
#include "GameInstance.h"

CEffectNotify::CEffectNotify(_float fTrackPosition, const _string& strEffectTag)
	: CAnimNotify{ fTrackPosition }
	, m_strEffectTag{ strEffectTag }
{
	m_wStrEffectTag = StringToWString(m_strEffectTag);
}

/* 함수 등록시 Lambda 호출 */
void CEffectNotify::Execute()
{
	if (m_EffectCallback)
		m_EffectCallback(m_wStrEffectTag);
}

json CEffectNotify::To_Json() const
{
	json colliderJson;
	colliderJson["NotifyType"] = "Effect";
	colliderJson["TrackPosition"] = m_fTrackPosition;
	colliderJson["EffectTag"] = m_strEffectTag;

	return colliderJson;
}

const _string& CEffectNotify::Get_NotifyTypeName() const
{
	static const _string typeName = "Effect";
	return typeName;
}

#ifdef _DEBUG
void CEffectNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
	ImGui::Text("Effect Tag : %s", m_strEffectTag.c_str());
}
#endif // _DEBUG

CEffectNotify* CEffectNotify::From_Json(const json& colliderJson)
{
	CEffectNotify* pInstance = new CEffectNotify(
		colliderJson["TrackPosition"],
		colliderJson["EffectTag"]
	);

	return pInstance;
}

void CEffectNotify::Free()
{
	CAnimNotify::Free();
}





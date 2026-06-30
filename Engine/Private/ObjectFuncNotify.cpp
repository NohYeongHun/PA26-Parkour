#include "EnginePch.h"
#include "ObjectFuncNotify.h"

CObjectFuncNotify::CObjectFuncNotify(_float fTrackPosition, const _string& strObjectTag)
	: CAnimNotify{ fTrackPosition }
	, m_strObjectTag{ strObjectTag }
{
	m_wStrObjectTag = StringToWString(m_strObjectTag);
}

void CObjectFuncNotify::Execute()
{
	if (m_ObjectCallback)
		m_ObjectCallback(m_wStrObjectTag);
}

json CObjectFuncNotify::To_Json() const
{
	json colliderJson;
	colliderJson["NotifyType"] = "Object";
	colliderJson["TrackPosition"] = m_fTrackPosition;
	colliderJson["ObjectTag"] = m_strObjectTag;

	return colliderJson;
}

const _string& CObjectFuncNotify::Get_NotifyTypeName() const
{
	static const _string typeName = "Object";
	return typeName;
}

#ifdef _DEBUG
void CObjectFuncNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
	ImGui::Text("Object Tag : %s", m_strObjectTag.c_str());
}
#endif // _DEBUG

CObjectFuncNotify* CObjectFuncNotify::From_Json(const json& objectJson)
{
	CObjectFuncNotify* pInstance = new CObjectFuncNotify(
		objectJson["TrackPosition"],
		objectJson["ObjectTag"]
	);

	return pInstance;
}

void CObjectFuncNotify::Free()
{
	CAnimNotify::Free();
}

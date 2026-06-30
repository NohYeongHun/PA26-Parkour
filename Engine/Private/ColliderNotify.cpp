#include "EnginePch.h"
#include "ColliderNotify.h"
#include "GameInstance.h"

CColliderNotify::CColliderNotify(_float fTrackPosition, const _string& strColliderTag, _bool IsActive)
	: CAnimNotify{ fTrackPosition}
	, m_strColliderTag { strColliderTag }
	, m_IsActive { IsActive }
{
	m_wStrColliderTag = StringToWString(m_strColliderTag);
}

void CColliderNotify::Execute()
{
	if (m_ColliderCallback)
		m_ColliderCallback(m_wStrColliderTag, m_IsActive);

}

json CColliderNotify::To_Json() const
{
	json colliderJson;
	colliderJson["NotifyType"] = "Collider";
	colliderJson["TrackPosition"] = m_fTrackPosition;
	colliderJson["ColliderTag"] = m_strColliderTag;
	colliderJson["IsActive"] = m_IsActive;

	return colliderJson;
}

const _string& CColliderNotify::Get_NotifyTypeName() const
{
	// ?명떚?뚯씠???щ윭媛??앹꽦?섎뒗??媛숈? typeName?대?濡?怨듭쑀?쒕떎.
	static const _string typeName = "Collider";
	return typeName;
}

#ifdef _DEBUG
void CColliderNotify::ImGui_Print()
{
	ImGui::Text("TrackPosition : %.2f", m_fTrackPosition);
	ImGui::Text("ColliderTag : %s", m_strColliderTag.c_str());
	if (m_IsActive)
		ImGui::Text("Collider Active");
	else
		ImGui::Text("Collider UnActive");
}
#endif // _DEBUG




//void CColliderNotify::Set_Callback(const function<void(const _wstring&, _bool)>& callback)
//{
//	m_ColliderCallback = callback;
//}

CColliderNotify* CColliderNotify::From_Json(const json& colliderJson)
{
	CColliderNotify* pInstance = new CColliderNotify(
		colliderJson["TrackPosition"],
		colliderJson["ColliderTag"],
		colliderJson["IsActive"]
	);

	return pInstance;
}

void CColliderNotify::Free()
{
	CAnimNotify::Free();
}





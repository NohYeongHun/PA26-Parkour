#include"EditorPch.h"
#include "Edit_MapEffectCollector.h"
#include"Level_Map.h"
#include"Event_Level.h"

CEdit_MapEffectCollector::CEdit_MapEffectCollector()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CEdit_MapEffectCollector::Initialize()
{
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Effect"), [this](const MAP_SAVE& event) {

		for (auto& pEffectInfo : m_EffectInfo)
		{
			event.File.write(reinterpret_cast<const char*>(&pEffectInfo.second.EffectTag), sizeof(_uint));
			event.File.write(reinterpret_cast<const char*>(&pEffectInfo.second.vPos), sizeof(_float4));
		}
		});

	return S_OK;
}

void CEdit_MapEffectCollector::Set_ImGuiOption()
{
	ImGui::Begin("Effect");
	ImGuiID ShaderId = ImGui::GetID("EffectCollector");
	ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
	ImGui::Text("Current Effect");

	for (auto& pEffectInfo : m_EffectInfo)
		if (ImGui::Button(to_string(pEffectInfo.first).c_str())) {
			m_iEffectIndex = pEffectInfo.first;
			m_eTempTag = pEffectInfo.second.EffectTag;
			m_vTempPos.float_3 = _float3(pEffectInfo.second.vPos.x, pEffectInfo.second.vPos.y, pEffectInfo.second.vPos.z);
		}
	ImGui::EndChildFrame();

	ImGui::Checkbox("Render Gizmo", &m_IsRenderGizmo);

	const _char* pEffectTag[] = { "Hearth_Fire","Hearth_Fire_2","CampFire" ,"Sonoro Statue"};

	if (ImGui::BeginCombo("Object_Type", pEffectTag[m_eTempTag]))
	{
		for (_uint i = 0; i < ENUM_CLASS(EFFECTTAG::END); ++i)
		{
			if (ImGui::Selectable(pEffectTag[i]))
			{
				m_eTempTag = static_cast<EFFECTTAG>(i);
			}
		}
		ImGui::EndCombo();
	}

	_int minus = -1;
	_uint plus = 1;
	ImGui::InputScalar("Instance Num Value : ", ImGuiDataType_U32, &m_iEffectIndex, &plus, &minus);

	ImGui::InputFloat3("Pos", m_vTempPos.arr);
	ImGui::SameLine();
	if (ImGui::Button("Get PickPos"))
		m_vTempPos.float_3 = _float3(CLevel_Map::m_vPickedPos.x, CLevel_Map::m_vPickedPos.y, CLevel_Map::m_vPickedPos.z);

	_float3 vScale = _float3(1.f, 1.f, 1.f);
	_float3 vRotation = _float3(1.f, 1.f, 1.f);
	if (m_IsRenderGizmo)
		m_pGameInstance->Use_Gizmo_Offset(&vScale, &vRotation, &m_vTempPos.float_3);
	if(ImGui::Button("Save"))
	{
		ETERNAL_EFFECT Desc{};
		m_EffectInfo[m_iEffectIndex++].EffectTag = m_eTempTag;
		m_EffectInfo[m_iEffectIndex++].vPos = _float4(m_vTempPos.float_3.x, m_vTempPos.float_3.y, m_vTempPos.float_3.z, 1.f);
	}

	if (ImGui::Button("Delete"))
		m_EffectInfo.erase(m_iEffectIndex++);
	
	ImGui::End();
}

void CEdit_MapEffectCollector::Map_Load(ETERNAL_EFFECT EffectDesc)
{
	m_EffectInfo.emplace(m_iEffectIndex, EffectDesc);
	m_iEffectIndex++;
}

CEdit_MapEffectCollector* CEdit_MapEffectCollector::Create()
{
	CEdit_MapEffectCollector* pInstance = new CEdit_MapEffectCollector();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Edit_MapEffectCollector");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapEffectCollector::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}

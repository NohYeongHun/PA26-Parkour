#include"EditorPch.h"
#include "Edit_LightManager.h"
#include"Edit_LightObject.h"
#include"Level_Map.h"
#include"Event_Level.h"

CEdit_LightManager::CEdit_LightManager()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CEdit_LightManager::Initialize()
{
	m_LightDesc.eType = LIGHT_DESC::POINT;
	m_LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	m_LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	m_LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_LightDesc.fRange = 1.f;
	m_LightDesc.vPosition = _float4(0.f, 0.f, 0.f, 1.f);

	m_pGameInstance->Subscribe<LIGHT_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Light_Create"), [this](const LIGHT_CREATE& event) {
		CEdit_LightObject* pObject = static_cast<CEdit_LightObject*>(event.pObject);
		event.iNumCreateIndex;
		m_pPickedLight = pObject;
		m_Lights.emplace(event.iNumCreateIndex, pObject);
		Safe_AddRef(pObject);
		});

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Light"), [this](const MAP_SAVE& event) {
		LIGHT_DESC SaveLightDesc;
		_uint Size = m_Lights.size();
		event.File.write(reinterpret_cast<const _char*>(&Size), sizeof(_uint));
		for (auto& pLight : m_Lights)
		{
			if (!pLight.second->IsActivate())
				continue;

			SaveLightDesc = *pLight.second->Save_Light();
			//구조체로 내보내는 거 안돼서 그냥 이따구로 함ㅇㅇ
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.eType), sizeof(_uint));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.fRange),sizeof(_float));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.vAmbient),sizeof(_float4));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.vDiffuse),sizeof(_float4));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.vDirection),sizeof(_float4));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.vPosition),sizeof(_float4));
			event.File.write(reinterpret_cast<const _char*>(&SaveLightDesc.vSpecular),sizeof(_float4));
		}
		});

	return S_OK;
}

void CEdit_LightManager::Set_ImGuiOption()
{
	ImGuiID Light = ImGui::GetID("Lights");
	ImGui::BeginChildFrame(Light, ImVec2(100, 200));
	ImGui::Text("Current Lights");

	for (auto iter = m_Lights.begin(); iter != m_Lights.end(); )
	{
		if (ImGui::Button(to_string(iter->first).c_str())) {
			m_pPickedLight = iter->second;
		}
		if (ImGui::IsItemHovered())
		{
			m_pGameInstance->Use_Gizmo(dynamic_cast<CTransform*>(iter->second->Get_Component(TEXT("Com_Transform"))));
		}

		if (!iter->second->IsActivate())
		{
			if (m_pPickedLight == iter->second)
				m_pPickedLight = nullptr;

			iter = m_Lights.erase(iter);
		}
		else
			++iter;
	}
	ImGui::EndChildFrame();

	Create_Light();




	if (m_pPickedLight)
		m_pPickedLight->Set_ImGuiOption();


}

void CEdit_LightManager::Create_Light()
{
	if (ImGui::Button("Create"))
	{
		CEdit_LightObject::MAP_LOAD Desc{};
		Desc.CopyDesc = nullptr;
		Desc.vWorldPos = CLevel_Map::m_vPickedPos;
		m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_GameObject_LightObject"), ENUM_CLASS(LEVEL::MAP), TEXT("Layer_Light"), &Desc);
	}
}

void CEdit_LightManager::Map_Load(LIGHT_DESC& Desc)
{

}

CEdit_LightManager* CEdit_LightManager::Create()
{
	CEdit_LightManager* pInstance = new CEdit_LightManager();
	pInstance->Initialize();
	return pInstance;
}

void CEdit_LightManager::Free()
{
	__super::Free();
	m_pPickedLight = nullptr;

	for (auto& pLight : m_Lights)
		Safe_Release(pLight.second);
	m_Lights.clear();

	Safe_Release(m_pGameInstance);
}
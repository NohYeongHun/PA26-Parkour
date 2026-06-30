#include"EditorPch.h"
#include "Edit_MonsterSpawnor.h"
#include"Map_Interface.h"
#include"Event_Level.h"

CEdit_MonsterSpawnor::CEdit_MonsterSpawnor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_MonsterSpawnor::CEdit_MonsterSpawnor(const CEdit_MonsterSpawnor& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_MonsterSpawnor::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Clone(nullptr)))
		return E_FAIL;

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Spawn"), [this](const MAP_SAVE& event) {
/*
		for (_uint i = 2; i < m_MonsterSpawn.size(); ++i)
		{
			auto& Map = m_MonsterSpawn[i];
			event.File.write(reinterpret_cast<const char*>(&Map.vMonsterSpawnorPos), sizeof(_float4));

			_uint iLength = {};
			event.File.write(reinterpret_cast<const char*>(&Map.vMonsterPos1.float4), sizeof(_float4));
			iLength = strlen(Map.szMonsterName1);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.szMonsterName1, iLength);

			event.File.write(reinterpret_cast<const char*>(&Map.vMonsterPos2.float4), sizeof(_float4));
			iLength = strlen(Map.szMonsterName2);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.szMonsterName2, iLength);

			event.File.write(reinterpret_cast<const char*>(&Map.vMonsterPos3.float4), sizeof(_float4));
			iLength = strlen(Map.szMonsterName3);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.szMonsterName3, iLength);

		}
		*/
		for (auto& Map : m_MonsterSpawn)
		{
			//event.File.write(reinterpret_cast<const char*>(&Map), sizeof(SPAWN_DESC));
			event.File.write(reinterpret_cast<const char*>(&Map.second.vMonsterSpawnorPos), sizeof(_float4));

			_uint iLength = {};
			event.File.write(reinterpret_cast<const char*>(&Map.second.vMonsterPos1.float4), sizeof(_float4));
			iLength = strlen(Map.second.szMonsterName1);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.second.szMonsterName1, iLength);
			
			event.File.write(reinterpret_cast<const char*>(&Map.second.vMonsterPos2.float4), sizeof(_float4));
			iLength = strlen(Map.second.szMonsterName2);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.second.szMonsterName2, iLength);
			
			event.File.write(reinterpret_cast<const char*>(&Map.second.vMonsterPos3.float4), sizeof(_float4));
			iLength = strlen(Map.second.szMonsterName3);
			event.File.write(reinterpret_cast<const char*>(&iLength), sizeof(_uint));
			event.File.write(Map.second.szMonsterName3, iLength);
		}
		});
    return S_OK;
}

HRESULT CEdit_MonsterSpawnor::Initialize_Clone(void* pArg)
{
	/*if(FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;*/

	Ready_Components(pArg);

    return S_OK;
}

void CEdit_MonsterSpawnor::Priority_Update(_float fTimeDelta)
{

}

void CEdit_MonsterSpawnor::Update(_float fTimeDelta)
{

}

void CEdit_MonsterSpawnor::Late_Update(_float fTimeDelta)
{

}

void CEdit_MonsterSpawnor::Set_ImGuiOption()
{

	ImGuiID ShaderId = ImGui::GetID("SavedSpawnor");
	ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));

	for (_uint i = 0; i < m_MonsterSpawn.size(); ++i)
	{
		if (ImGui::Button(to_string(i).c_str())) {
			m_iPickedSpawnNum = i;
		}
	}
	ImGui::EndChildFrame();

	ImGui::InputScalar("Instance SaveIndex: ", ImGuiDataType_U32, &m_iSpawnorIndex);

	//몬스터 여럿당 좌표 하나.
	ImGui::InputFloat3("Spawnor Pos : ", m_MonsterDesc.vMonsterSpawnorPos.arr, "%.2f");

	ImGui::InputFloat3("Monster1 Pos : ", m_MonsterDesc.vMonsterPos1.arr, "%.2f");
	ImGui::InputText("Monster1 Name : ", m_MonsterDesc.szMonsterName1, MAX_PATH);

	ImGui::InputFloat3("Monster2 Pos : ", m_MonsterDesc.vMonsterPos2.arr, "%.2f");
	ImGui::InputText("Monster2 Name : ", m_MonsterDesc.szMonsterName2, MAX_PATH);

	ImGui::InputFloat3("Monster3 Pos : ", m_MonsterDesc.vMonsterPos3.arr, "%.2f");
	ImGui::InputText("Monster3 Name : ", m_MonsterDesc.szMonsterName3, MAX_PATH);


	if (ImGui::Button("Add_Spawnor"))
		m_MonsterSpawn.emplace(m_iSpawnorIndex, m_MonsterDesc);
}

void CEdit_MonsterSpawnor::Map_Load(SPAWN_DESC& Desc)
{
	m_MonsterSpawn.emplace(m_iSpawnorIndex++, Desc);
}

void CEdit_MonsterSpawnor::Ready_Components(void* pArg)
{

}

CEdit_MonsterSpawnor* CEdit_MonsterSpawnor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MonsterSpawnor* pInstance = new CEdit_MonsterSpawnor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_MonsterSpawnor");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MonsterSpawnor::Clone(void* pArg)
{
	CEdit_MonsterSpawnor* pInstance = new CEdit_MonsterSpawnor(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_MonsterSpawnor (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MonsterSpawnor::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);
	//Safe_Release(m_pMapInterface);
}
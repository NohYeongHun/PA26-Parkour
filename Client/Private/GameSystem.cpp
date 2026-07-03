#include "ClientPch.h"
#include "GameSystem.h"
#include "Parser.h"
#include "MouseController.h"
#include "Sonoro_Manager.h"



IMPLEMENT_SINGLETON(CGameSystem)

CGameSystem::CGameSystem()
{
}

void CGameSystem::Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pMouseController = CMouseController::Create();
	ASSERT_CRASH(m_pMouseController);

	m_pParser = CParser::Create(pDevice, pContext);
	ASSERT_CRASH(m_pParser);

	m_pSonoro_Manager = CSonoro_Manager::Create();
	ASSERT_CRASH(m_pSonoro_Manager);
}

void CGameSystem::Update(_float fTimeDelta)
{
}

void CGameSystem::Clear_Resource()
{
	Clear_TriggerCallBack();
}

void CGameSystem::Register_Mouse(CMouse* pMouse)
{
	m_pMouseController->Register_Mouse(pMouse);
}
void CGameSystem::Set_MouseFix(_bool isFix)
{
	m_pMouseController->Set_MouseFix(isFix);
}
_bool CGameSystem::IsFix()
{
	return m_pMouseController->IsFix();
}

void CGameSystem::Ready_Prototype_Map(const _char* pDataFilePath, LEVEL eLevel, const _char* pModelFilePath)
{
	return m_pParser->Ready_Prototype_Map(pDataFilePath, eLevel, pModelFilePath);
}

void CGameSystem::Clone_MapObjects(LEVEL eLevel)
{
	m_pParser->Clone_MapObjects(eLevel);
}

#pragma region SONORO
void CGameSystem::Set_Sonora_LightDesc(SONORA eType, const LIGHT_DESC& Desc)
{
	m_pSonoro_Manager->Set_Sonora_LightDesc(eType, Desc);
}
_bool* CGameSystem::Add_To_Management(OBJECTTYPE eType, CMapObject_Sonoro* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool* CGameSystem::Add_To_Management(OBJECTTYPE eType, CMapObject_NonSonoro* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool* CGameSystem::Add_To_Management(INSTANCETYPE eType, CMapObject_Instance* pObjects, _bool** SonoroMode)
{
	return m_pSonoro_Manager->Add_To_Management(eType, pObjects, SonoroMode);
}

_bool CGameSystem::IsSonoro()
{
	return m_pSonoro_Manager->IsSonoro();
}

#pragma endregion


void CGameSystem::Clear_TriggerCallBack()
{
	for (auto& TriggerVector : m_TriggerEvents)
		TriggerVector.second.clear();
	m_TriggerEvents.clear();
}

void CGameSystem::Release_System()
{
	Safe_Release(m_pParser);
	Safe_Release(m_pMouseController);
	Safe_Release(m_pSonoro_Manager);
	Release();
}

void CGameSystem::Free()
{
	__super::Free();
}

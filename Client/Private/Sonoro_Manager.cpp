#include "ClientPch.h"
#include "Sonoro_Manager.h"
#include "MapObject_Sonoro.h"
#include "MapObject_NonSonoro.h"
#include "MapObject_Instance.h"

CSonoro_Manager::CSonoro_Manager()
	:m_pGameInstance(CGameInstance::GetInstance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CSonoro_Manager::Initialize()
{
	m_vUpSpeed = _float4(0.f, 23.6f, 0.f, 0.f);

	m_EnterSonoro = TEXT("소노라 진입하기");
	m_ExitSonoro = TEXT("소노라 떠나기");;
	
	return S_OK;
}

_bool* CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_Sonoro* pObjects, _bool** SonoroMode)
{
	if (eType == OBJECTTYPE::SONORA)
	{
		{
			lock_guard<mutex> lock(m_Mutex);
			m_SonoroObjects.push_back(pObjects);
			*SonoroMode = &m_SonoroRigidActive;
			Safe_AddRef(pObjects);
		}
		return &m_SonoroRender;
	}

	CRASH("Failed");
	return nullptr;
}

_bool* CSonoro_Manager::Add_To_Management(OBJECTTYPE eType, CMapObject_NonSonoro* pObjects, _bool** SonoroMode)
{
	if (eType == OBJECTTYPE::NONSONORA || eType == OBJECTTYPE::NONSONORA_FLOOR)
	{
		{
			lock_guard<mutex> lock(m_Mutex);
			m_NonSonoroObjects.push_back(pObjects);
			*SonoroMode = &m_SonoroRigidActive;
			Safe_AddRef(pObjects);
		}

		return &m_SonoroRender;
	}

	CRASH("Failed");
	return nullptr;
}

_bool* CSonoro_Manager::Add_To_Management(INSTANCETYPE eType, CMapObject_Instance* pObjects, _bool** SonoroMode)
{
	{
		lock_guard<mutex> lock(m_Mutex);
		m_Instance.push_back(pObjects);
		*SonoroMode = &m_SonoroRigidActive;
		Safe_AddRef(pObjects);
		return &m_SonoroRender;
	}

	return nullptr;
}

void CSonoro_Manager::Update(_float fTimeDelta)
{
	if (!m_IsUpdate)
		return;

	m_fTriggerdTime += fTimeDelta;

	if (m_SonoroRigidActive)
	{
		// None -> Sonora
		if (m_fTriggerdTime >= 5.f)
		{
			m_pGameInstance->Update_LightDesc(TEXT("Test"), m_LightDesc[ENUM_CLASS(SONORA::SONORA)]);
			m_SonoroRender = !m_SonoroRender;
			m_IsUpdate = !m_IsUpdate;
			m_fTriggerdTime = 0.f;

			for (auto& pObject : m_SonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);

			//코드 변경 가능성 高

			for (auto& pObject : m_NonSonoroObjects)
				pObject->ReturnPos();

			_uint iSoundChannel = m_pGameInstance->Register_Channel();
			m_pGameInstance->Play_Sound(TEXT("SonoroBegin0"), iSoundChannel, 0.1f);
			m_pGameInstance->Return_Channel(iSoundChannel);


		}
		else
			for (auto& pObject : m_NonSonoroObjects)
				pObject->Turn_Sonoro(XMLoadFloat4(&m_vUpSpeed) * fTimeDelta, m_fTriggerdTime);
	}
	else
	{
		// Sonora -> None

		if (m_fTriggerdTime >= 4.f)
		{
			m_pGameInstance->Update_LightDesc(TEXT("Test"), m_LightDesc[ENUM_CLASS(SONORA::NONE)]);
			for (auto& pObject : m_NonSonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);

			for (auto& pObject : m_SonoroObjects)
				pObject->Change_Collision_Layer(m_SonoroRigidActive);
			m_SonoroRender = !m_SonoroRender;
			m_IsUpdate = !m_IsUpdate;
			m_fTriggerdTime = 0.f;
			_uint iSoundChannel = m_pGameInstance->Register_Channel();
			m_pGameInstance->Play_Sound(TEXT("SonoroBegin0"), iSoundChannel, 0.1f);
			m_pGameInstance->Return_Channel(iSoundChannel);
		}
	}
}



const _tchar* CSonoro_Manager::Get_SonoroText()
{
	return m_SonoroRender ?
		m_ExitSonoro.c_str() :
		m_EnterSonoro.c_str();
}

void CSonoro_Manager::Clear_Resource()
{
	for (auto& pObject : m_SonoroObjects)
		Safe_Release(pObject);
	m_SonoroObjects.clear();

	for (auto& pObject : m_NonSonoroObjects)
		Safe_Release(pObject);
	m_NonSonoroObjects.clear();

	for (auto& pInstance : m_Instance)
		Safe_Release(pInstance);
	m_Instance.clear();
}


CSonoro_Manager* CSonoro_Manager::Create()
{
	CSonoro_Manager* pInstance = new CSonoro_Manager();
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Sonoro_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSonoro_Manager::Free()
{
	for (auto& pObject : m_SonoroObjects)
		Safe_Release(pObject);
	m_SonoroObjects.clear();

	for (auto& pObject : m_NonSonoroObjects)
		Safe_Release(pObject);
	m_NonSonoroObjects.clear();

	for (auto& pInstance : m_Instance)
		Safe_Release(pInstance);
	m_Instance.clear();

	Safe_Release(m_pGameInstance);
}

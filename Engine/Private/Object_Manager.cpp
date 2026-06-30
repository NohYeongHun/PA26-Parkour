#include "EnginePch.h"
#include "Object_Manager.h"
#include "GameInstance.h"

#include "GameObject.h"
#include "Layer.h"

CObject_Manager::CObject_Manager()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CObject_Manager::Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg)
{
	if (m_iNumLevel <= iLayerLevelID)
		return E_FAIL;

	CGameObject* pClone = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::GAMEOBJECT, pArg));
	if (nullptr == pClone)
		return E_FAIL;

	{
		lock_guard<mutex> lock(m_Mutex);
		CLayer* pLayer = Find_Layer(iLayerLevelID, strLayerTag);
		if (nullptr == pLayer)
		{
			pLayer = CLayer::Create();
			m_Layers[iLayerLevelID].emplace(strLayerTag, pLayer);
		}
		pLayer->Add_GameObject(pClone);
	}

	return S_OK;
}

HRESULT CObject_Manager::Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, CGameObject* pObject)
{
	if (m_iNumLevel <= iLayerLevelID)
		return E_FAIL;

	{
		lock_guard<mutex> lock(m_Mutex);
		CLayer* pLayer = Find_Layer(iLayerLevelID, strLayerTag);
		if (nullptr == pLayer)
		{
			pLayer = CLayer::Create();
			m_Layers[iLayerLevelID].emplace(strLayerTag, pLayer);
		}
		pLayer->Add_GameObject(pObject);
	}

	return S_OK;
}

CComponent* CObject_Manager::Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag)
{
	CLayer* pLayer = Find_Layer(iLayerLevelID, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;
	return pLayer->Get_Component(iGameObjectIndex, strComponentTag);
}

HRESULT CObject_Manager::Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _bool isTimeStop)
{
	CLayer* pLayer = Find_Layer(iLayerLevelID, strLayerTag);
	if (nullptr == pLayer)
		return E_FAIL;

	pLayer->Change_TimeRate(fTimeRatio, isTimeStop);

	return S_OK;
}

HRESULT CObject_Manager::Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _float fDuration)
{
	CLayer* pLayer = Find_Layer(iLayerLevelID, strLayerTag);
	if (nullptr == pLayer)
		return E_FAIL;

	pLayer->Change_TimeRate(fTimeRatio, fDuration);

	return S_OK;
}

HRESULT CObject_Manager::Initialize(_uint iNumLevel)
{
	if (0 >= iNumLevel)
		return E_FAIL;
	m_iNumLevel = iNumLevel;
	m_Layers = new LAYERS[m_iNumLevel];

	return S_OK;
}

void CObject_Manager::Priority_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevel; ++i)
	{
		for (auto& Pair : m_Layers[i])
			(Pair.second)->Priority_Update(fTimeDelta);
	}
}

void CObject_Manager::Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevel; ++i)
	{
		for (auto& Pair : m_Layers[i])
			(Pair.second)->Update(fTimeDelta);
	}
}

void CObject_Manager::Late_Update(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevel; ++i)
	{
		for (auto& Pair : m_Layers[i])
			(Pair.second)->Late_Update(fTimeDelta);
	}
}

HRESULT CObject_Manager::Clear_Resource(_uint iClearLevelID)
{
	if (m_iNumLevel <= iClearLevelID)
		return E_FAIL;

	for (auto& Pair : m_Layers[iClearLevelID])
		Safe_Release(Pair.second);
	m_Layers[iClearLevelID].clear();

	return S_OK;
}

CLayer* CObject_Manager::Find_Layer(_uint iLayerLevelID, const _wstring& strLayerTag)
{
	if (m_iNumLevel <= iLayerLevelID)
		return nullptr;

	auto iter = m_Layers[iLayerLevelID].find(strLayerTag);
	if (iter == m_Layers[iLayerLevelID].end())
		return nullptr;

	return iter->second;
}

CObject_Manager* CObject_Manager::Create(_uint iNumLevel)
{
	CObject_Manager* pInstance = new CObject_Manager();

	if (FAILED(pInstance->Initialize(iNumLevel)))
	{
		MSG_BOX("Failed to Create : Object_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObject_Manager::Free()
{
	__super::Free();

	for (size_t i = 0; i < m_iNumLevel; ++i)
	{
		for (auto& Pair : m_Layers[i])
			Safe_Release(Pair.second);
		m_Layers[i].clear();
	}
	Safe_Delete_Array(m_Layers);

	Safe_Release(m_pGameInstance);
}

#include "EnginePch.h"
#include "Prototype_Manager.h"

#include "GameObject.h"
#include "Component.h"

CPrototype_Manager::CPrototype_Manager()
{
}

HRESULT CPrototype_Manager::Initialize(_uint iNumLevel)
{
    if (0 >= iNumLevel)
        return E_FAIL;
    m_iNumLevel = iNumLevel;

    m_Prototypes = new PROTOTYPES[m_iNumLevel];

    return S_OK;
}

HRESULT CPrototype_Manager::Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	if (nullptr == pPrototype || m_iNumLevel <= iPrototypeLevelID)
		CRASH("None Prototype");

    auto iter = m_Prototypes[iPrototypeLevelID].find(strPrototypeTag);
    if (iter != m_Prototypes[iPrototypeLevelID].end())
    {
        Safe_Release(pPrototype);
        return E_FAIL;
	}

	{
		lock_guard<mutex> lock(m_Mutex);
		m_Prototypes[iPrototypeLevelID].emplace(strPrototypeTag, pPrototype);
	}

    return S_OK;
}

void CPrototype_Manager::Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag)
{
	auto iter = m_Prototypes[iPrototypeLevelID].find(strPrototypeTag);
	if (iter != m_Prototypes[iPrototypeLevelID].end())
	{
		Safe_Release(iter->second);
		m_Prototypes[iPrototypeLevelID].erase(iter);
	}
}

CBase* CPrototype_Manager::Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg)
{
    if(m_iNumLevel <= iPrototypeLevelID)
        return nullptr;

    auto iter = m_Prototypes[iPrototypeLevelID].find(strPrototypeTag);
    if (iter == m_Prototypes[iPrototypeLevelID].end())
        return nullptr;

    CBase* pPrototype = iter->second;
    CBase* pClone = { nullptr };
    switch (eType)
    {
    case PROTOTYPE::GAMEOBJECT:
        pClone = static_cast<CGameObject*>(pPrototype)->Clone(pArg);
        break;
    case PROTOTYPE::COMPONENT:
        pClone = static_cast<CComponent*>(pPrototype)->Clone(pArg);
        break;
    }

    return pClone;
    }

HRESULT CPrototype_Manager::Clear_Resource(_uint iClearLevelID)
{
    if (m_iNumLevel <= iClearLevelID)
        return E_FAIL;

    for (auto& Pair : m_Prototypes[iClearLevelID])
    {
        Safe_Release(Pair.second);
    }
    m_Prototypes[iClearLevelID].clear();

    return S_OK;
}

CPrototype_Manager* CPrototype_Manager::Create(_uint iNumLevel)
{
    CPrototype_Manager* pInstance = new CPrototype_Manager();

    if (FAILED(pInstance->Initialize(iNumLevel)))
    {
        MSG_BOX("Failed to Create : Prototype_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPrototype_Manager::Free()
{
    __super::Free();

    for (size_t i = 0; i < m_iNumLevel; ++i)
    {
        for (auto& Pair : m_Prototypes[i])
            Safe_Release(Pair.second);
        m_Prototypes[i].clear();
    }
    Safe_Delete_Array(m_Prototypes);
}

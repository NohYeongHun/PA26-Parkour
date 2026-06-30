#include "EnginePch.h"
#include "Level_Manager.h"
#include "GameInstance.h"

#include "Level.h"

CLevel_Manager::CLevel_Manager()
    : m_pGameInstance { CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLevel_Manager::Open_Level(_uint iCurrentLevel, CLevel* pCurrentLevel)
{
    if (nullptr == pCurrentLevel)
        return E_FAIL;

    Safe_Release(m_pCurrentLevel);
    m_pCurrentLevel = pCurrentLevel;

    return S_OK;
}

void CLevel_Manager::Update_Level(_float fTimeDelta)
{
    if (nullptr == m_pCurrentLevel)
        return;
    m_pCurrentLevel->Update(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
    if (nullptr == m_pCurrentLevel)
        return S_OK;
    m_pCurrentLevel->Render();
	return S_OK;
}

HRESULT CLevel_Manager::Clear_CurrentLevel_Resources(_uint iNextLevel)
{
	HRESULT hr = S_OK;

	// 이전 레벨의 Resource 정리
	if (nullptr != m_pCurrentLevel)
		hr = m_pGameInstance->Clear_Resource(m_iCurrentLevel);

	// Level 생성 시점에 해당 레벨을 CurrentLevel로 갖고 있는다.
	m_iCurrentLevel = iNextLevel;
	m_pGameInstance->Model_Manager_Change_Level(m_iCurrentLevel);
	return hr;
}

CLevel_Manager* CLevel_Manager::Create()
{
    return new CLevel_Manager();
}

void CLevel_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pCurrentLevel);
    Safe_Release(m_pGameInstance);
}

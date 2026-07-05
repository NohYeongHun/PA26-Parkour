#include "ClientPch.h"
#include "Level_Loading.h"
#include "GameSystem.h"
#include "Event_Level.h"

// ========Loader========
#include "Loader_Test.h"

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevel)
{
    m_eNextLevel = eNextLevel;

    if (FAILED(Ready_Prototype()))
        return E_FAIL;

    if (FAILED(Ready_Event()))
        return E_FAIL;

    if (FAILED(Ready_LoadingThread()))
        return E_FAIL;

    if (FAILED(Ready_GameObject()))
        return E_FAIL;

    return S_OK;
}

void CLevel_Loading::Update(_float fTimeDelta)
{
	if (false == m_isFinished)
	{
		m_pLoader->Update(fTimeDelta);
	}

	if (false == m_isFinished && true == m_pGameInstance->IsWorkFinish())
	{
		_float fProgress = m_pLoader->Get_Progress();
		if (0.99f < fProgress)
		{
			m_isFinished = true;
			cout << "Loading End" << endl;
			CHANGE_LEVEL_EVENT event{ m_eNextLevel, false };
			m_pGameInstance->Publish(ENUM_CLASS(STATIC::STATIC), TEXT("Event_Change_Level"), event);
		}
	}
}

void CLevel_Loading::Render()
{
	//static _uint i = 0;
	//i++;
	//cout << "[CLevel_Loading::Render] Render Called! : " << i << endl;
}

HRESULT CLevel_Loading::Ready_Prototype()
{
    return S_OK;
}

HRESULT CLevel_Loading::Ready_Event()
{
    m_pGameInstance->Subscribe<LOADING_END_EVENT>(ENUM_CLASS(STATIC::NONE), TEXT("Event_Loading_End"), [this](const LOADING_END_EVENT& event) {
            if(false == m_isFinished)
                m_isFinished = event.isFinish;
        });

    return S_OK;
}

HRESULT CLevel_Loading::Ready_LoadingThread()
{
    switch (m_eNextLevel)
    {
	case LEVEL::TEST:
		m_pLoader = CLoader_Test::Create(m_pDevice, m_pContext);
		break;
    }

	ASSERT_CRASH(m_pLoader);

	this_thread::sleep_for(chrono::seconds(1));

    return S_OK;
}

HRESULT CLevel_Loading::Ready_GameObject()
{
    return S_OK;
}

CLevel_Loading* CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevel)
{
    CLevel_Loading* pInstance = new CLevel_Loading(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eNextLevel)))
    {
        MSG_BOX("Failed to Create : Level_Loading");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Loading::Free()
{
    __super::Free();
    Safe_Release(m_pLoader);
	Safe_Release(m_pGameSystem);
}

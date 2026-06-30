#include "EnginePch.h"
#include "EventBus.h"

CEventBus::CEventBus()
{
}

HRESULT CEventBus::Initialize()
{
    return S_OK;
}

CEventBus* CEventBus::Create()
{
    CEventBus* pInstance = new CEventBus();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : EventBus");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEventBus::Free()
{
    __super::Free();

    for (size_t i = 0; i < 2; ++i)
        m_Listener[i].clear();
}

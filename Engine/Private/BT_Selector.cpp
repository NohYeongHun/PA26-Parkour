#include "EnginePch.h"
#include "BT_Selector.h"
#include "BlackBoard.h"

CBT_Selector::CBT_Selector()
    :CBT_Node{}
{
#ifdef _DEBUG
    m_iType = 1;
#endif // _DEBUG
}

CBT_Selector::CBT_Selector(const CBT_Selector& Prototype)
    :CBT_Node{ Prototype }
    ,m_Children { Prototype.m_Children }
{
    for (auto& Pattern : m_Children)
        Safe_AddRef(Pattern);
}

HRESULT CBT_Selector::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBT_Selector::Initialize_Clone(void* pArg)
{
    return S_OK;
}

CBT_Node::BT_STATE CBT_Selector::tick(CGameObject* pGameObject, class CBlackBoard* pBlackBoard)
{
    BT_STATE eState{};
    for (auto& child : m_Children)
    {
        eState = child->tick(pGameObject, pBlackBoard);
        if (BT_STATE::FAILURE != eState)
            return eState;
    }

    return BT_STATE::FAILURE;
}

CBT_Selector* CBT_Selector::Create()
{
    CBT_Selector* pInstance = new CBT_Selector();
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBT_Selector");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBT_Node* CBT_Selector::Clone(void* pArg)
{
    CBT_Selector* pInstance = new CBT_Selector(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBT_Selector");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBT_Selector::Free()
{
    for (auto& child : m_Children)
        Safe_Release(child);
    m_Children.clear();
    __super::Free();
}



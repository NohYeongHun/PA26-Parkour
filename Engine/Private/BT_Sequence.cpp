#include "EnginePch.h"
#include "BT_Sequence.h"
#include "BlackBoard.h"

CBT_Sequence::CBT_Sequence()
    :CBT_Node{}
{
#ifdef _DEBUG
    m_iType = 2;
#endif // _DEBUG

}

CBT_Sequence::CBT_Sequence(const CBT_Sequence& Prototype)
    :CBT_Node{ Prototype }
    ,m_Children { Prototype.m_Children }
{
    for (auto& child : m_Children)
        Safe_AddRef(child);
}

HRESULT CBT_Sequence::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBT_Sequence::Initialize_Clone(void* pArg)
{
	return S_OK;
}

CBT_Node::BT_STATE CBT_Sequence::tick(CGameObject* pGameObject, class CBlackBoard* pBlackBoard)
{
    BT_STATE eState{};
    for (auto& child : m_Children)
    {
        eState = child->tick(pGameObject, pBlackBoard);
        if (BT_STATE::SUCCESS != eState)
            return eState;
    }

    return BT_STATE::SUCCESS;
}

CBT_Sequence* CBT_Sequence::Create()
{
    CBT_Sequence* pInstance = new CBT_Sequence();
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CBT_Sequence");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBT_Node* CBT_Sequence::Clone(void* pArg)
{
    CBT_Sequence* pInstance = new CBT_Sequence(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBT_Sequence");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBT_Sequence::Free()
{
    for (auto& child : m_Children)
        Safe_Release(child);
    m_Children.clear();
    __super::Free();
}

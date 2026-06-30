#include "EnginePch.h"
#include "AnimTransition.h"

CAnimTransition::CAnimTransition()
{
}

HRESULT CAnimTransition::Initialize_Prototype(json& jsonParser, CONDITION Func)
{
    m_strNextState =  jsonParser["To"];
    //for(auto& strCondition : jsonParser["Conditions"])

    m_iPriority = jsonParser["Priority"];
    m_iTargetState = jsonParser["Target State"];
    m_fTargetTrackPos = jsonParser["Transit Target Pos"];
    m_fTransitEnablePos = jsonParser["Transit Enable Pos"];

    m_Condition = Func;
    return S_OK;
}

_bool CAnimTransition::Is_Transit(const _uint* pOwnerState, _string& strNextState, _float& fTargetTrackPos)
{
    //if(*pOwnerState == m_iTargetState)
    if(m_Condition && m_Condition(pOwnerState, m_iTargetState))
    {
        //for(auto& Func : m_Conditions)
        //    if(Func(pOwnerState))
        //    {
        //        return false;
        //    }

        strNextState = m_strNextState;
        fTargetTrackPos = m_fTargetTrackPos;
        return true;
    }
    return false;
}

CAnimTransition* CAnimTransition::Create(json& jsonParser, CONDITION Func)
{
    CAnimTransition* pInstance = new CAnimTransition();
    if(FAILED(pInstance->Initialize_Prototype(jsonParser, Func)))
    {
        MSG_BOX("Failed to Created : CAnimTransition");
        Safe_Release(pInstance);
    }
    return pInstance;
}

#ifdef _DEBUG
CAnimTransition* CAnimTransition::Create()
{
    CAnimTransition* pInstance = new CAnimTransition();
    return pInstance;
}
#endif // _DEBUG

void CAnimTransition::Free()
{
    __super::Free();
}

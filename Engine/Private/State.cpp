#include "EnginePch.h"
#include "State.h"
#include "StateMachine.h"


HRESULT CState::Initialize(CGameObject* pOwner)
{
    return S_OK;
}

void CState::OnEnter(void* pArg)
{
}


void CState::OnUpdate(_float fTimeDelta)
{

}


void CState::OnExit()
{

}

void CState::Change_State(CStateMachine* pStateMachine, _uint iCategory, _uint iSubState)
{
    pStateMachine->Change_State(iCategory, iSubState);
}




void CState::Free()
{
    CBase::Free();
}

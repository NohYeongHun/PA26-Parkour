#include "EnginePch.h"
#include "StateMachine.h"
#include "State.h"


#pragma region
CStateMachine::CStateMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CStateMachine::CStateMachine(const CStateMachine& Prototype)
    : CComponent(Prototype)
    , m_States{ Prototype.m_States }
    , m_CurrentStateKey{ Prototype.m_CurrentStateKey }
    , m_pCurrentState{ Prototype.m_pCurrentState }
{
    for (auto& pair : m_States)
        Safe_AddRef(pair.second);

    Safe_AddRef(m_pCurrentState);
}



HRESULT CStateMachine::Initialize_Prototype()
{

    return S_OK;
}

HRESULT CStateMachine::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CStateMachine::Update(_float fTimeDelta)
{
    // 현재 State Update
    // State 내부에서 Check_StateTransition()을 통해 Change_State 호출
    if (nullptr != m_pCurrentState)
    {
        m_pCurrentState->OnUpdate(fTimeDelta);
    }
}

void CStateMachine::Change_State(_uint iCategory, _uint iSubState, void* pArg)
{
    Change_State(StateKey(iCategory, iSubState), pArg);
}

void CStateMachine::Change_State(const StateKey& key, void* pArg)
{
    auto iter = m_States.find(key);
    if (iter == m_States.end())
        return;

    if (nullptr != m_pCurrentState)
    {
		m_iPrevAnimIndex = m_pCurrentState->Get_CurrentAnimIndex();
        m_pCurrentState->OnExit();
        m_PrevStateKey = m_CurrentStateKey;
		
    }

    m_CurrentStateKey = key;
    m_pCurrentState = iter->second;
    m_pCurrentState->OnEnter(pArg); // pArg를 전달하며 Enter 호출
	m_iCurAnimIndex = m_pCurrentState->Get_CurrentAnimIndex();
}

void CStateMachine::Add_State(_uint iCategory, _uint iSubState, CState* pState)
{
    Add_State(StateKey(iCategory, iSubState), pState);
}

void CStateMachine::Add_State(const StateKey& key, CState* pState)
{
    m_States.emplace(key, pState);
}

void CStateMachine::Exit_State()
{
	if (nullptr != m_pCurrentState)
		m_pCurrentState->OnExit();
}


#pragma endregion


CStateMachine* CStateMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CStateMachine* pInstance = new CStateMachine(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CStateMachine");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CStateMachine::Clone(void* pArg)
{
    CStateMachine* pInstance = new CStateMachine(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Clone Failed : CStateMachine");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CStateMachine::Free()
{
    CComponent::Free();
    for (auto& pair : m_States)
        Safe_Release(pair.second);
    m_States.clear();
}

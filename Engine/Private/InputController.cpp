#include "EnginePch.h"
#include "InputController.h"
#include "GameInstance.h"

CInputController::CInputController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext}
{
}

CInputController::CInputController(const CInputController& Prototype)
    : CComponent (Prototype)
{
}

HRESULT CInputController::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CInputController::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CInputController::Update()
{
    // 1. Update Key Input
	if (!m_IsBlock)
		Update_KeyInput();
	else
		m_KeyInput = 0; // KeyInput이 아무것도 안눌린 상태로.
    
}

void CInputController::Update_KeyInput()
{
    m_PrevKeyInput = m_KeyInput;

    // 1.
    m_KeyInput = 0;
    KEYSTATE eState = { KEYSTATE::END };

    // 2. 키보드 매핑 로직.
    for (_uint i = 0; i < m_KeyboardMappings.size(); ++i)
    {
		// 벡터에 등록된 키 매핑을 하나씩 꺼냅니다.
        const auto& keyMapping = m_KeyboardMappings[i]; 
		// 꺼낸 키 매핑 로직으로 키 상태(ubyte)를 얻어와서 현재 입력되었는지 확인합니다.
        eState = m_pGameInstance->Get_DIKeyState(keyMapping.second);

		// PRESS or Down 상태라면 KeyInput 플래그에 해당 키 매핑 플래그를 설정합니다.
        if (eState == KEYSTATE::PRESS || eState == KEYSTATE::DOWN)
            m_KeyInput |= keyMapping.first;
    }

    // 3. 마우스 매핑 로직
    for (_uint i = 0; i < m_MouseMappings.size(); ++i)
    {
        const auto& keyMapping = m_MouseMappings[i];
        eState = m_pGameInstance->Get_DIMouseState(keyMapping.second);
        if (eState == KEYSTATE::PRESS || eState == KEYSTATE::DOWN)
            m_KeyInput |= keyMapping.first;
    }

}

_bool CInputController::Check_AnyInput(_uint eKeyInput, KEYSTATE eState) const
{
    _uint iFlag = static_cast<_uint>(eKeyInput);
    if (eState == KEYSTATE::PRESS)
        return (m_KeyInput & iFlag) != 0;

    if (eState == KEYSTATE::DOWN)
        return !(m_PrevKeyInput & iFlag) && (m_KeyInput & iFlag);

    if (eState == KEYSTATE::UP)
        return (m_PrevKeyInput & iFlag) && !(m_KeyInput & iFlag);

    return false;
}

_bool CInputController::Check_AllInput(_uint eKeyInput, KEYSTATE eState) const
{
    _uint iFlag = static_cast<_uint>(eKeyInput);

    if (eState == KEYSTATE::PRESS)
        return (m_KeyInput & iFlag) == iFlag;

    if (eState == KEYSTATE::DOWN)
        return !((m_PrevKeyInput & iFlag) == iFlag) && ((m_KeyInput & iFlag) == iFlag);

    if (eState == KEYSTATE::UP)
        return ((m_PrevKeyInput & iFlag) == iFlag) && !((m_KeyInput & iFlag) == iFlag);

    return false;
}


/*
* Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::W), DIK_W));
*/
void CInputController::Register_KeyBoardKeyInput(_uint iKey, _ubyte keyboardValue)
{
    m_KeyboardMappings.emplace_back(make_pair(iKey, keyboardValue));
}

/*
* Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB));
*/
void CInputController::Register_MouseKeyInput(_uint iKey, MOUSEKEYSTATE mouseValue)
{
    m_MouseMappings.emplace_back(make_pair(iKey, mouseValue));
}

CInputController* CInputController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CInputController* pInstance = new CInputController(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : CInputController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CInputController::Clone(void* pArg)
{
    CInputController* pInstance = new CInputController(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : CInputController");
        Safe_Release(pInstance);
    }

    return pInstance;
}

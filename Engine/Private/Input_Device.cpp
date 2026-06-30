#include "EnginePch.h"
#include "Input_Device.h"

CInput_Device::CInput_Device()
{
}

KEYSTATE CInput_Device::Get_DIKeyState(_ubyte byKeyID)
{
	if (m_OldKeyStates[byKeyID] >= 0 && m_KeyStates[byKeyID] < 0)
		return KEYSTATE::DOWN;
	if (m_OldKeyStates[byKeyID] < 0 && m_KeyStates[byKeyID] < 0)
		return KEYSTATE::PRESS;
	if (m_OldKeyStates[byKeyID] < 0 && m_KeyStates[byKeyID] >= 0)
		return KEYSTATE::UP;

	return KEYSTATE::END;
}

KEYSTATE CInput_Device::Get_DIMouseState(MOUSEKEYSTATE eState)
{
	_ubyte byMouseKey = static_cast<_byte>(eState);
	if (m_OldMouseStates.rgbButtons[byMouseKey] <= 0 && m_MouseStates.rgbButtons[byMouseKey] > 0)
		return KEYSTATE::DOWN;
	if (m_OldMouseStates.rgbButtons[byMouseKey] > 0 && m_MouseStates.rgbButtons[byMouseKey] > 0)
		return KEYSTATE::PRESS;
	if (m_OldMouseStates.rgbButtons[byMouseKey] > 0 && m_MouseStates.rgbButtons[byMouseKey] <= 0)
		return KEYSTATE::UP;

	return KEYSTATE::END;
}

HRESULT CInput_Device::Initialize(HINSTANCE hInst, HWND hWnd)
{
	if (FAILED(DirectInput8Create(hInst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pInputSDK, nullptr)))
		return E_FAIL;

	// KeyBoard
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr)))
		return E_FAIL;
	if (FAILED(m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard)))
		return E_FAIL;
	if (FAILED(m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;
	if (FAILED(m_pKeyBoard->Acquire()))
		return E_FAIL;

	// Mouse
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;
	if (FAILED(m_pMouse->SetDataFormat(&c_dfDIMouse)))
		return E_FAIL;
	if (FAILED(m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		return E_FAIL;
	if (FAILED(m_pMouse->Acquire()))
		return E_FAIL;

	return S_OK;
}

void CInput_Device::Update()
{
	memcpy(&m_OldKeyStates, &m_KeyStates, sizeof(_byte) * 256);
	m_pKeyBoard->GetDeviceState(256, &m_KeyStates);
	memcpy(&m_OldMouseStates, &m_MouseStates, sizeof(DIMOUSESTATE));
	m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_MouseStates);
}

CInput_Device* CInput_Device::Create(HINSTANCE hInst, HWND hWnd)
{
	CInput_Device* pInstance = new CInput_Device();

	if (FAILED(pInstance->Initialize(hInst, hWnd)))
	{
		MSG_BOX("Failed to Create : Input_Device");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInput_Device::Free()
{
	__super::Free();

	Safe_Release(m_pKeyBoard);
	Safe_Release(m_pMouse);
	Safe_Release(m_pInputSDK);
}

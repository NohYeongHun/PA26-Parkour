#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CInput_Device final : public CBase
{
private:
	explicit CInput_Device();
	virtual ~CInput_Device() = default;

public:
	KEYSTATE Get_DIKeyState(_ubyte byKeyID);
	KEYSTATE Get_DIMouseState(MOUSEKEYSTATE eState);
	_long Get_DIMouseMove(MOUSEMOVESTATE eState)
	{
		return *(reinterpret_cast<_int*>(&m_MouseStates) + ENUM_CLASS(eState));
	}

public:
	HRESULT							Initialize(HINSTANCE hInst, HWND hWnd);
	void								Update();

private:
	LPDIRECTINPUT8				m_pInputSDK = { nullptr };

	LPDIRECTINPUTDEVICE8		m_pKeyBoard = { nullptr };
	LPDIRECTINPUTDEVICE8		m_pMouse= { nullptr };

private:
	_byte								m_KeyStates[256] = {};
	_byte								m_OldKeyStates[256] = {};
	DIMOUSESTATE				m_MouseStates = {};
	DIMOUSESTATE				m_OldMouseStates = {};

public:
	static		CInput_Device*		Create(HINSTANCE hInst, HWND hWnd);
	virtual		void					Free() override;
};

NS_END
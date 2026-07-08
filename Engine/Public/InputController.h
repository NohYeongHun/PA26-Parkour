#pragma once
#include "Component.h"

NS_BEGIN(Engine)
// 인풋과 관련된 처리 
// 키입력을 등록합니다.
class ENGINE_DLL CInputController final : public CComponent
{
public:
	explicit CInputController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CInputController(const CInputController& Prototype);
	virtual ~CInputController() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual void Update();

public:
	
	void Set_BlockInput(_bool IsBlock) { m_IsBlock = IsBlock; }
	void Update_KeyInput();
	

public:
	_bool Check_AnyInput(_uint iKeyInput, KEYSTATE eState = KEYSTATE::PRESS) const; // OR
	_bool Check_AllInput(_uint iKeyInput, KEYSTATE eState = KEYSTATE::PRESS) const; // AND

	
public:
	void Register_KeyBoardKeyInput(_uint iKey, _ubyte keyboardValue);
	void Register_MouseKeyInput(_uint iKey, MOUSEKEYSTATE mouseValue);

private:
	_uint m_PrevKeyInput = {};
	_uint m_KeyInput = {};
	_bool m_IsBlock = {};
	vector<pair<_uint, _ubyte>> m_KeyboardMappings = {};
	vector<pair<_uint, MOUSEKEYSTATE>> m_MouseMappings = {};

public:
	static CInputController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;


};
NS_END


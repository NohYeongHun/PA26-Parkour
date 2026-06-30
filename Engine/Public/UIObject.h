#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC {
		_float fX{}, fY{}, fSizeX{}, fSizeY{};
	} UI_DESC;

protected:
	explicit CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CUIObject(const CUIObject& Prototype);
	virtual ~CUIObject() = default;

public:
	virtual		HRESULT				Initialize_Prototype();
	virtual		HRESULT				Initialize_Clone(void* pArg);
	virtual		void				Priority_Update(_float fTimeDelta);
	virtual		void				Update(_float fTimeDelta);
	virtual		void				Late_Update(_float fTimeDelta);
	virtual		void				Render();

	virtual		_bool				Check_OnInteract(_uint iEventInteractType, _uint iInstanceIndex = 0);	// returns local variable that presents event state.

protected:
	virtual		void				OnEvent(_uint iEventType);			// calls event functions

	virtual		void				Update_InputState();						// updates local variable that presents event state.
	virtual		_bool				Check_IsInSpace();

public:
	virtual		void				Set_Active(_bool isActive)		{ m_isActivate = isActive; };
	virtual		_bool				Get_Active() { return m_isActivate; };


protected:
	_float					m_fX{}, m_fY{}, m_fSizeX{}, m_fSizeY{};
	_float4x4				m_ViewMatrix = {};
	_float4x4				m_ProjMatrix = {};

	_float					m_iWinSizeX{}, m_iWinSizeY{};

	_bool					m_isHovered = false;
	_bool					m_isClicked = false;

protected:
	HRESULT Begin();


public:
	virtual		CGameObject*		Clone(void* pArg) = 0;
	virtual		void				Free() override;
};

NS_END
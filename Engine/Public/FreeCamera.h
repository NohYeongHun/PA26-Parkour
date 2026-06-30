#pragma once
#include "Camera.h"

NS_BEGIN(Engine)

class CFreeCamera final : public CCamera
{
private:
	explicit CFreeCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CFreeCamera(const CFreeCamera& Prototype);
	virtual ~CFreeCamera() = default;

public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void				Priority_Update(_float fTimeDelta);
	virtual		void				Update(_float fTimeDelta);
	virtual		void				Late_Update(_float fTimeDelta);
	virtual		void				Render();

private:
	_float							m_fSpeed = {};
	_float3						m_vPosition = {};

public:
	static		CFreeCamera*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*	Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END
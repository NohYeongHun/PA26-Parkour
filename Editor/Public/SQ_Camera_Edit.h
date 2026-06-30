#pragma once
#include "Camera.h"

NS_BEGIN(Editor)

class CSQ_Camera_Edit final : public CCamera
{
private:
	explicit CSQ_Camera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSQ_Camera_Edit(const CSQ_Camera_Edit& Prototype);
	virtual ~CSQ_Camera_Edit() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;

	// Pooling Spawn CallBack
	virtual		void				Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	vector<SCENE_CAMERA_FRAME>		m_Frames;
	_int											m_iFrameIndex = { 0 };
	_float											m_fTrackPosition = {};
	_float											m_fTrackPerSec = { 10.f };

	_float											m_fStartFrame = {};
	_float											m_fEndFrame = {};

	_float											m_fRatio = {};

private:
	void							Lerp_Quat();
	void							Spline();

public:
	static CSQ_Camera_Edit*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void					Free() override;
};

NS_END
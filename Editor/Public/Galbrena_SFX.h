#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Engine)

class CVIBuffer_Rect_Instance;

NS_END

NS_BEGIN(Editor)

class CGalbrena_SFX final : public CEdit_ScreenEffect
{
private:
	CGalbrena_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrena_SFX(const CGalbrena_SFX& Prototype);
	virtual ~CGalbrena_SFX() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;

	virtual		void		Play() override;
	virtual		void		Stop() override;
	virtual		void		Reset() override;

private:
	
	_float2					m_vEffectTime = {};
	_float					m_fCurrentTime = {};

	_float3					m_vColor = { _float3(1.f, 1.f, 1.f) };

	_float2					m_vDegree = {};
	_float2					m_vDrawRadians = {};
	_float					m_fCurrentRadian = {};

	_float2					m_vScale = {};
	_float					m_fCurrentScale = {1.f};

	_float2					m_vPivot = {};
	_float2					m_vPivotRate = {};

	CVIBuffer_Rect_Instance*	m_pVIBuffer = { nullptr };
	CShader*					m_pShader = { nullptr };

	_uint					m_iSlashIndex = {};
	_float					m_fSizeX[5] = { 1.f, 1.f, 1.f, 1.f, 1.f};
	_float					m_fSizeY[5] = { 1.f, 1.f, 1.f, 1.f, 1.f };

	_float					m_fRadians[5] = {};

private:
	void						Ready_Instance();
	VTXINSTANCE_RECT			Make_Instance(_float2 vPivot, _float fSizeX, _float fSizeY, _float fRotateZ);

public:
	static CGalbrena_SFX*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END
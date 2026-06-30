#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Editor)

class CGalbrena_Blur final : public CEdit_ScreenEffect
{
private:
	typedef struct tagRadialData
	{
		_float fMinDistance;
		_float fMaxDistance;
		_float fLengthScale;
		_float fPadding0;
		_float2 vPivot;
		_float2 fPadding1;
	}RADIAL_DATA;

private:
	CGalbrena_Blur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrena_Blur(const CGalbrena_Blur& Prototype);
	virtual ~CGalbrena_Blur() = default;

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
	CVIBuffer_Rect*		m_pVIBuffer = { nullptr };
	CShader*			m_pShader = { nullptr };
	CTexture*			m_pNoiseTexture = { nullptr };

	ID3D11Buffer*		m_pBuffer = { nullptr };

	RADIAL_DATA			m_RadialData = {};

	_float2				m_vEffectTime = {};
	_float				m_fCurrentTime = {};

	_float2				m_vLengthScale = {};
	_float2				m_vMaxDistance = {};

	_float2				m_vRadialTime = {};

	_float2				m_vReversTime[2] = {};
	_bool				m_IsRevers = {};

private:
	HRESULT					Ready_Components();

public:
	static CGalbrena_Blur*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END
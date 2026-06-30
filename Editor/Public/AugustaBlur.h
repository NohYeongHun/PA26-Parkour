#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Editor)

class CAugustaBlur final : public CEdit_ScreenEffect
{
private:
	typedef struct tagSlashData
	{
		_float2 vSlashPoint0;
		_float2 vSlashPoint1;
		_float fOffset;
		_float fIntensity;
		_float2 Padding;
	}SLASH_DATA;

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
	CAugustaBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAugustaBlur(const CAugustaBlur& Prototype);
	virtual ~CAugustaBlur() = default;

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
	SLASH_DATA				m_SlashData = {};
	RADIAL_DATA				m_RadialData = {};

	_float2					m_vEffectTime = {};
	_float					m_fCurrentTime = {};

	CTexture*				m_pNoiseTexture = { nullptr };

	ID3D11Buffer*			m_pBuffer = { nullptr };

	CVIBuffer_Rect* m_pVIBuffer_Rect = { nullptr };
	CShader* m_pShader = { nullptr };
private:
	HRESULT					Ready_Buffer();

public:
	static CAugustaBlur*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg);
	virtual void			Free() override;
};

NS_END
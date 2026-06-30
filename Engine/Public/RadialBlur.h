#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CRadialBlur final : public CBlur
{
private:
	typedef struct tagRadialData
	{
		_float fMinDistance;
		_float fMaxDistance;
		_float fLengthScale;
		_float fPadding;
		_float2 vPivot;
		_float2 fPadding1;
	}RADIAL_DATA;

private:
	CRadialBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContrext);
	virtual ~CRadialBlur() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual void		Update(_float fTimeDelta) override;
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

	virtual void		Enter();
	virtual void		Exit();

	void				Setting_Radial(_float2 vCenterUV, _float2 vDistanceRange, _float fRadialIntensity);
	void				Setting_Radial(_fvector  vCenterPos, _float2 vDistanceRange, _float fRadialIntensity);

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};

	_float				m_fCurLength = {};
	_float				m_fTargetLength = {};

	RADIAL_DATA			m_RadialData = {};

	ID3D11SamplerState* m_pClampSampler = { nullptr };

public:
	static CRadialBlur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
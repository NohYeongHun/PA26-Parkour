#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

class CBloom final : public CBlur
{
private:
	typedef struct tagBloomUpData {
		_float2  vSize;
		_float	fIntensity;
		_float   Padding;
	}BLOOM_UP_DATA;

private:
	CBloom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBloom() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	_uint				m_iBloomWeight = {};
	_float				m_fBoolIntensity = {};

public:
	static CBloom*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

class CScreenBlur final : public CBlur
{
private:
	CScreenBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CScreenBlur() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};

public:
	static CScreenBlur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
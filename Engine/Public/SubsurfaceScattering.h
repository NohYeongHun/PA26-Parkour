#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CSubsurfaceScattering final : public CBlur
{
private:
	CSubsurfaceScattering(ID3D11Device* pDevice, ID3D11DeviceContext* pContrext);
	virtual ~CSubsurfaceScattering() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual void		Update(_float fTimeDelta) override;
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};

public:
	static CSubsurfaceScattering*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void					Free() override;
};

NS_END
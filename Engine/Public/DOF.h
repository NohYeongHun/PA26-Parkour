#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

class CDOF final : public CBlur
{
private:
	CDOF(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDOF() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;
	void				Setting_DOF(_float3 vCenterPos, _float fRange);

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	
	_float3				m_vCenterPos = {};
	_float				m_fDofRange = {};
	_float				m_fDofScale = {};

private:
	HRESULT				Bind_Resources(CShader* pShader);

public:
	static CDOF*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
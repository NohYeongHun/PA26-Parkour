#pragma once
#include "SFX.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CVIBuffer_Rect;

class CSSAO final : public CSFX
{
private:
	typedef struct tagSSaoBlurData {
		_float  fSSAO_MinDepthDistance;
		_float	fWidth;
		_float	fHeight;
		_float  Paddingblur;
	}SSAO_BLUR_DATA;

private:
	CSSAO(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSSAO() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	_uint				m_fWinSizeX = {};
	_uint				m_fWinSizeY = {};


	CTexture*			m_pNoiseTexture = { nullptr };
	vector<_vector>		m_SSAO_SampleVector;
	_uint				m_iNumKernel = {};
	_float				m_fRadius = {};
	_float				m_fMaxDistance = {};
	_float				m_fOutDistance = {};
	_float				m_fSSAO_MinDepthDistance = {};

private:
	HRESULT				Bind_Resources(CShader* pShader);

	HRESULT				Ready_NoiseTexture();
	HRESULT				Ready_SSAO_SampleVector();

public:
	static CSSAO*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
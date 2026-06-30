#pragma once
#include "SFX.h"

NS_BEGIN(Engine)


class CBlur abstract : public CSFX
{
protected:
	typedef struct tagBlurData {
		_float2  vSize;
		_int	 iRadius;
		_float   Padding;
	}BLUR_DATA;

	typedef vector<pair<_int, vector<_float>>> BLUR_WEIGHTS;

protected:
	CBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBlur() = default;

public:
	virtual HRESULT						Initialize();
	virtual HRESULT						Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) { return S_OK; }

protected:
	_uint								m_iNumWeights = {};
	BLUR_WEIGHTS						m_Weights;
	vector<ID3D11Buffer*>				m_WeightBuffers;
	vector<ID3D11ShaderResourceView*>	m_WeightSRVs;

protected:
	HRESULT								Add_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iBlurWeight = 1);

private:
	HRESULT								Ready_BlurWeights();
	HRESULT								Create_BlurBuffer(const vector<_float>& Weights, _uint iRadius);

public:
	virtual void		Free() override;
};

NS_END
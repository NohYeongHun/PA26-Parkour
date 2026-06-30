#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Spectrum : public CVIBuffer
{
public:
	typedef struct tagBufferSpectrumDesc
	{
		_int	MaxSamples;
		_float	fMaxTrailLength;
		_float  fSize;
	}VB_SPECTRUM_DESC;

private:
	explicit CVIBuffer_Spectrum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Spectrum(const CVIBuffer_Spectrum& Prototype);
	virtual ~CVIBuffer_Spectrum() = default;

public:
	virtual		HRESULT				Initialize_Prototype(VB_SPECTRUM_DESC* pDesc);
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	virtual		HRESULT				Render() override;
	virtual		HRESULT				Bind_Resources() override;

public:
	void Update_Spectrum(deque<SAMPLE_DESC>& vSamples, _int SampleCount, const _float4* vCamPos);

	void Update_SmoothSpectrum(deque<SAMPLE_DESC>& vSamples, _int SampleCount, const _float4* vCamPos);

private:
	_int			m_iMaxSamples = {};
	_float			m_fSize = {};
	_float			m_fMaxTrailLength = 1.5f;

	_int			m_iVtxCount = {};

public:
	static		CVIBuffer_Spectrum* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, VB_SPECTRUM_DESC* pDesc);
	virtual		CComponent* Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END

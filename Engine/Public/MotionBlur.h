#pragma once
#include "Blur.h"

NS_BEGIN(Engine)

class CShader;
class CVIBuffer_Rect;

class CMotionBlur final : public CBlur
{
private:
	typedef struct tagMotionBlurData {
		_float fLimitVelocity;
		_float fLimitDepth;
		_float fLengthScale;
		_float fSampleDepthBias;
	}MOTION_BLUR_DATA;

private:
	CMotionBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMotionBlur() = default;

public:
	virtual HRESULT		Initialize(_uint iWinSizeX, _uint iWinSizeY);
	virtual void		Update(_float fTimeDelta) override;
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

	virtual void		Enter();
	virtual void		Exit();

#ifdef _DEBUG
	void				Set_Motion(_float fLimitVelocity, _float fLimitDepth, _float fLengthScale) { m_MotionBlurData.fLimitVelocity = fLimitVelocity, m_MotionBlurData.fLimitDepth = fLimitDepth, 
		m_MotionBlurData.fLengthScale = fLengthScale; }
#endif

private:
	_uint				m_iWinSizeX = {};
	_uint				m_iWinSizeY = {};
	_float				m_fWinSizeX = {};
	_float				m_fWinSizeY = {};
	
	_float				m_fTargetLength = {};
	_float				m_fCurLength = {};

	MOTION_BLUR_DATA	m_MotionBlurData = {};

	ID3D11SamplerState* m_pClampSampler = { nullptr };

public:
	static CMotionBlur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual void		Free() override;
};

NS_END
#pragma once
#include "SFX.h"

NS_BEGIN(Engine)

class CWater final : public CSFX
{
private:
	CWater(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CWater() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void		Update(_float fTimeDelta) override;
	virtual HRESULT		Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) override;

#ifdef _DEBUG
public:
	void					Set_SSR(_float fMinStep, _float fMaxStep, _float fStartOffset)
	{
		m_fMinStepSize = fMinStep, m_fMaxStepSize = fMaxStep, m_fStartOffset = fStartOffset;
	}
#endif

private:
	_float				m_fMinStepSize = {};
	_float				m_fMaxStepSize = {};
	_float				m_fStartOffset = {};

	_float				m_fMaxDepth;
	_float				m_fMinTickness;
	_float				m_fMaxTickness;

public:
	static CWater*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free();
};

NS_END
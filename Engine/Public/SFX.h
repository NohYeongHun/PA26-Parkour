#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CVIBuffer_Rect;
class CShader;

class CSFX abstract : public CBase
{
protected:
	CSFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSFX() = default;

public:
	virtual HRESULT			Initialize() { return S_OK; }
	virtual void			Update(_float fTimeDelta) {}
	virtual HRESULT			Render(CVIBuffer_Rect* pVIBuffer, CShader* pShader) { return S_OK; }
	virtual void			Enter() {}
	virtual void			Exit() {}

	void					Set_Intensity(_float fIntensity) { m_fIntensity = fIntensity; }

protected:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance*			m_pGameInstance = { nullptr };
	_float					m_fIntensity = {};

public:
	virtual void Free() override;
};

NS_END
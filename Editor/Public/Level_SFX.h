#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CSFX_Interface;

class CLevel_SFX final : public CLevel
{
private:
	explicit CLevel_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_SFX() = default;

public:
	virtual HRESULT			Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	HRESULT					Ready_Light();
	HRESULT					Ready_Interface();
	HRESULT					Ready_TestObjects();

private:
	CSFX_Interface*			m_pSFX_Interface = { nullptr };

public:
	static		CLevel_SFX* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void		Free() override;
};

NS_END
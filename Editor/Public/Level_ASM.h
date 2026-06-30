#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CASM_Interface;

class CLevel_ASM final : public CLevel
{
private:
	explicit CLevel_ASM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_ASM() = default;

public:
	virtual HRESULT			Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	HRESULT					Ready_Interface();
	HRESULT					Ready_TestObjects();

private:
	CASM_Interface*			m_pASM_Interface = { nullptr };

public:
	static		CLevel_ASM*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END
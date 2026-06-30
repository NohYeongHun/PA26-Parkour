#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CShader_Interface;

class CLevel_Shader final : public CLevel
{
private:
	explicit CLevel_Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Shader() = default;

public:
	virtual HRESULT			Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	HRESULT					Ready_Light();
	HRESULT					Ready_Interface();
	HRESULT					Ready_TestObjects();

private:
	CShader_Interface*		m_pShader_Interface = { nullptr };
	LIGHT_DESC				m_TestLight = {};

public:
	static		CLevel_Shader*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void			Free() override;
};

NS_END
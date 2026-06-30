#pragma once
#include "Base.h"

NS_BEGIN(Editor)

class CInterface_Edit abstract : public CBase
{
protected:
	explicit CInterface_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CInterface_Edit() = default;

public:
	virtual		HRESULT			Initialize() { return S_OK; }

protected:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

public:
	virtual		void				Free() override;
};

NS_END
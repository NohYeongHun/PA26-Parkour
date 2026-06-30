#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;

class ENGINE_DLL CLevel abstract : public CBase
{
protected:
	explicit CLevel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual ~CLevel() = default;

public:
	virtual HRESULT		Initialize();
	virtual void			Update(_float fTimeDelta);
	virtual void			Render();

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CGameInstance*			m_pGameInstance = { nullptr };

public:
	virtual void Free() override;
};

NS_END
#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CComponent abstract : public CBase
{
protected:
	explicit CComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CComponent(const CComponent& Prototype);
	virtual ~CComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
	virtual HRESULT		Render();


protected:
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };

	_bool								m_isClone = { false };

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END
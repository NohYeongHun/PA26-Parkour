#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CEnvironmentQueryComponent final : public Engine::CComponent
{
protected:
	explicit CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype);
	virtual ~CEnvironmentQueryComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


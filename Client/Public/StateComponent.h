#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CStateComponent final : public Engine::CComponent
{
protected:
	explicit CStateComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStateComponent(const CStateComponent& Prototype);
	virtual ~CStateComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	void Move(_fvector vWorldDir, _float fTimeDelta, _float fSpeed);


public:
	static	CStateComponent* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


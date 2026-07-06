#pragma once
#include "Component.h"

NS_BEGIN(Client)
/*
* 메시의 물리적 형태가 어떤 파쿠르 동작을 허용할지를 판단하는 컴포넌트.
*/
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
	void Execute(_float fTimeDelta);

private:
	class CTransform* m_pOwnerTransformCom = { nullptr };

public:
	static	CEnvironmentQueryComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


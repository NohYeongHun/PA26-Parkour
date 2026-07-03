#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CMovementComponent final : public Engine::CComponent
{
protected:
	explicit CMovementComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMovementComponent(const CMovementComponent& Prototype);
	virtual ~CMovementComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	ACTORDIR Calculate_Direction(const class CInputController* pInputController) const;
	_vector Calculate_Move_Direction(const class CSpringCamera* pSpringCamera, ACTORDIR eDir) const;
	void Move_Direction(const CInputController* pInputController, const CSpringCamera* pSpringCamera, _float fTimeDelta, _float fSpeed) const;
	void Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed);

	
private:
	class CTransform* m_pTransformCom = { nullptr }; // 약한 참조.


public:
	static	CMovementComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


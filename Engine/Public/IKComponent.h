#pragma once
#include "Component.h"


NS_BEGIN(Engine)
// IK들을 관리할 컴포넌트
class ENGINE_DLL CIKComponent final : public CComponent
{
private:
	explicit CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKComponent(const CIKComponent& Prototype);
	virtual ~CIKComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;


public:
	static CIKComponent* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


#pragma once
#include "Component.h"


NS_BEGIN(Engine)
// IK들을 관리할 컴포넌트
class ENGINE_DLL CIKComponent final : public CComponent
{
public:
	typedef struct tagIKComponentDesc : CComponent::COMPONENT_DESC
	{
		class CModel*	  pOwnerModelCom = { nullptr };
		class CTransform* pOwnerTransformCom = { nullptr };
	}IKCOMPONENT_DESC;

private:
	explicit CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKComponent(const CIKComponent& Prototype);
	virtual ~CIKComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;


private:
	// 참조용 컴포넌트 (약한 참조)
	class CModel* m_pModelCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };


private:
	vector<class CIKSolver*> m_Solvers{};


public:
	static CIKComponent* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END


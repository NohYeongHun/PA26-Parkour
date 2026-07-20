#include "EnginePch.h"
#include "IKComponent.h"

CIKComponent::CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CIKComponent::CIKComponent(const CIKComponent& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CIKComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKComponent::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKComponent::Render()
{
	return S_OK;
}

CIKComponent* CIKComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKComponent* pInstance = new CIKComponent(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKComponent");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CIKComponent::Clone(void* pArg)
{
	CIKComponent* pClone = new CIKComponent(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKComponent (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKComponent::Free()
{
	__super::Free();
}

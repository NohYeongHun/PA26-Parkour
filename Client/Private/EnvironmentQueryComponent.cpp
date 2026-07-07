#include "ClientPch.h"
#include "EnvironmentQueryComponent.h"


CEnvironmentQueryComponent::CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CEnvironmentQueryComponent::CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype)
	: CComponent ( Prototype )
{
}

HRESULT CEnvironmentQueryComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEnvironmentQueryComponent::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (nullptr == m_pOwner)
		return E_FAIL;

	m_pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pOwnerTransformCom)
		return E_FAIL;

	m_pOwnerColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pOwnerColliderCom)
		return E_FAIL;
	

	return S_OK;
}

void CEnvironmentQueryComponent::Execute(_float fTimeDelta)
{
	m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), m_pOwnerTransformCom->Get_Quaternion(),
		m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset(),
		m_pOwnerTransformCom->Get_State(STATE::LOOK), 4.f, ENUM_CLASS(COLLISIONLAYER::PARKOUR), m_OutHits);

}

// 1. 1차적으로 Owner가 가지고 있는 Shape를 통한 충돌 가능성을 파악합니다.
void CEnvironmentQueryComponent::Broad_Phase(_float fTimeDelta)
{

}

// 2. Ray Cast 를 통한 지형 지물의 높이 정보를 판단합니다.
void CEnvironmentQueryComponent::Narrow_Phase(_float fTimeDelta)
{

}

CEnvironmentQueryComponent* CEnvironmentQueryComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvironmentQueryComponent* pInstance = new CEnvironmentQueryComponent(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CEnvironmentQueryComponent");
		Safe_Release(pInstance);
	}

	return pInstance;
}

Engine::CComponent* CEnvironmentQueryComponent::Clone(void* pArg)
{
	CEnvironmentQueryComponent* pClone = new CEnvironmentQueryComponent(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : EnvironmentQueryComponent (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEnvironmentQueryComponent::Free()
{
	__super::Free();
	m_pOwnerTransformCom = nullptr;
	m_pOwnerColliderCom = nullptr;
}

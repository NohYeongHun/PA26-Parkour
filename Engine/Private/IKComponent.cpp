#include "EnginePch.h"
#include "IKComponent.h"
#include "Model.h"
#include "Transform.h"
#include "IKSovler_CCD.h"
#include "IKSovler_Fabrik.h"
#include "IKSovler_TwoBone.h"

CIKComponent::CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CIKComponent::CIKComponent(const CIKComponent& Prototype)
	: CComponent(Prototype)
	, m_Solvers { Prototype.m_Solvers }
{
	for (auto& pSovler : m_Solvers)
		Safe_AddRef(pSovler);
}

// Solver 생성.
HRESULT CIKComponent::Initialize_Prototype()
{
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::TWO_BONE)] = CIKSovler_TwoBone::Create(m_pDevice, m_pContext);
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::CCD)]		= CIKSovler_CCD::Create(m_pDevice, m_pContext);
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::FABRIK)]	= CIKSovler_Fabrik::Create(m_pDevice, m_pContext);
	return S_OK;
}

HRESULT CIKComponent::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	const IKCOMPONENT_DESC* pDesc = static_cast<IKCOMPONENT_DESC*>(pArg);

	m_pModelCom = dynamic_cast<CModel*>(pDesc->pOwnerModelCom);
	ASSERT_CRASH(m_pModelCom);

	m_pTransformCom = dynamic_cast<CTransform*>(pDesc->pOwnerTransformCom);
	ASSERT_CRASH(m_pTransformCom);
	

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
	m_pModelCom = nullptr; 
	m_pTransformCom = nullptr;

	for (auto& pSolver : m_Solvers)
		Safe_Release(pSolver);
}

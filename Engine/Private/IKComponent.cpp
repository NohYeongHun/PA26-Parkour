#include "EnginePch.h"
#include "IKComponent.h"
#include "Model.h"
#include "Transform.h"
#include "IKSovler_CCD.h"
#include "IKSovler_Fabrik.h"
#include "IKSovler_TwoBone.h"
#include "Bone.h"

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

_uint CIKComponent::Register_Goal(const _string& strName, EIKSOLVER_TYPE eSolver, const vector<_string>& BoneNames)
{
	// 1. 중복 방지.
	auto it = m_GoalHandles.find(strName);
	if (it != m_GoalHandles.end())
		return it->second;

	// 2. 본 이름을 통한 인덱스 찾기.
	IK_GOAL goal{};
	goal.strName = strName;
	goal.eSolver = eSolver;

	for (auto& name : BoneNames)
	{
		_uint iBoneIndex = Find_BoneIndex(name.c_str());
		ASSERT_CRASH(iBoneIndex != UINT_MAX);
		goal.Chain.BoneChain.push_back(iBoneIndex);
	}

	// 3. push한 위치가 handle값
	_uint iGoal = static_cast<_uint>(m_Goals.size());
	m_Goals.push_back(goal);

	// 4. Map 등록
	m_GoalHandles.emplace(strName, iGoal);
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
	m_pModelCom->Get_Bones();
	return S_OK;
}

_uint CIKComponent::Find_BoneIndex(const char* pBoneName)
{
	const vector<CBone*>& Bones = m_pModelCom->Get_Bones();
	for (_uint idx = 0; idx < static_cast<_uint>(Bones.size()); ++idx)
	{
		if (0 == strcmp(pBoneName, Bones[idx]->Get_Name()))
			return idx;
	}

	return UINT_MAX;
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

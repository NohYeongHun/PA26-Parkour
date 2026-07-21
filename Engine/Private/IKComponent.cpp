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

void CIKComponent::Set_Target(const _string& strGoal, _vector& vWorldPos)
{
	_uint iGoalHandle = m_GoalHandles[strGoal];
	m_Goals[iGoalHandle].vCurTargetPos = vWorldPos;
}


void CIKComponent::Register_Goals(const _string& strFolderPath)
{
	// 1. 외부 FolderPath를 받아 Json을 파싱한다.
	namespace fs = std::filesystem;
	if (!fs::exists(strFolderPath)) return;

	for (const auto& entry : fs::directory_iterator(strFolderPath))
	{
		if (entry.is_directory() || entry.path().extension() != ".json")
			continue;

		std::ifstream file(entry.path());
		if (!file.is_open()) continue;

		json j;
		try { file >> j; }
		catch (...) { continue; }   // 파싱 실패 스킵

		_string        strName = j.value("GoalName", string(""));
		EIKSOLVER_TYPE eSolver = To_SolverType(j.value("Solver", string("")));
		if (strName.empty() || eSolver == EIKSOLVER_TYPE::END) continue;

		// 2. 본 체인 (root→end)
		vector<_string> boneNames;
		if (j.contains("BoneChain"))
			for (const auto& b : j["BoneChain"])
				boneNames.push_back(b.get<string>());
		if (boneNames.empty()) continue;

		// 3. 등록
		_uint iGoal = Register_Goal(strName, eSolver, boneNames);

		// 4. Register_Goal이 안 담는 나머지 채우기
		IK_GOAL& goal = m_Goals[iGoal];
		if (j.contains("poleVector") && j["poleVector"].size() == 3) // Two_Bone Solver 특화
			goal.Chain.vPoleVector = XMVectorSet(j["poleVector"][0], j["poleVector"][1], j["poleVector"][2], 0.f);
		goal.Chain.fPosWeight = j.value("posWeight", 1.f);
		goal.Chain.fRotWeight = j.value("rotWeight", 1.f);
	}
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

	return iGoal;
}

void CIKComponent::Begin_Goal(const _string& strGoalName, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec)
{
}

void CIKComponent::End_Goal(const _string& strGoalName, _float fBlendSec)
{
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

void CIKComponent::Update(_float fTimeDelta)
{
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

EIKSOLVER_TYPE CIKComponent::To_SolverType(const _string& strSolverType)
{
	if (strSolverType == "TwoBone")
		return EIKSOLVER_TYPE::TWO_BONE;

	if (strSolverType == "CCD")
		return EIKSOLVER_TYPE::CCD;

	if (strSolverType == "Fabrik")
		return EIKSOLVER_TYPE::FABRIK;

	return EIKSOLVER_TYPE::END;
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

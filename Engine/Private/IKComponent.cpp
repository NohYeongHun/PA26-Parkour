#include "EnginePch.h"
#include "IKComponent.h"
#include "Model.h"
#include "Transform.h"
#include "IKSolver_CCD.h"
#include "IKSolver_Fabrik.h"
#include "IKSolver_TwoBone.h"
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

// 등록된 Bone Chain을 사용 가능한 상태로 만듭니다.
void CIKComponent::Begin_Target(const _string& strTarget, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec)
{
	auto iter = m_TargetHandles.find(strTarget);
	if (iter == m_TargetHandles.end())
		return;

	IK_TARGET& Target = m_Targets[iter->second];


	Target.isEnable = true;
	Target.isTargetSet = false;
	Target.eMode = eMode;
	Target.Chain.fPosWeight = fPosWeight;
	Target.Chain.fRotWeight = fRotWeight;
	Target.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
	Target.fTargetWeight = 1.f;
	
}

void CIKComponent::End_Target(const _string& strTarget, _float fBlendSec)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end()) return;
	IK_TARGET& target = m_Targets[it->second];

	target.fTargetWeight = 0.f;
	target.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
}

void CIKComponent::Set_Target(const _string& strGoal, _fvector vWorldPos, _fvector vNormal)
{
	auto iter = m_TargetHandles.find(strGoal);
	if (iter == m_TargetHandles.end())
		return;

	IK_TARGET& target = m_Targets[iter->second];
	_matrix matWorldInv = m_pTransformCom->Get_WorldMatrix_Inv();
	_vector vModelPos = XMVector3TransformCoord(vWorldPos, matWorldInv); // 모델 스페이스 변환
	_vector vModelNormal = XMVector3TransformNormal(vNormal, matWorldInv); // 모델 스페이스 변환 => 아직 미사용

	target.Chain.vTargetPos = vModelPos;
	target.vCurTargetPos = vModelPos;
	target.vTargetNormal = vModelNormal;
	target.isTargetSet = true;
}


// LeftArm, RightArm 등 특정 본 체인을 게임 시작 시 json으로 파싱해둡니다.
void CIKComponent::Register_Targets(const _string& strFolderPath)
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

		_string        strName = j.value("TargetName", string(""));
		EIKSOLVER_TYPE eSolver = To_SolverType(j.value("Solver", string("")));
		if (strName.empty() || eSolver == EIKSOLVER_TYPE::END) continue;

		// 2. 본 체인 (root→end)
		vector<_string> boneNames;
		if (j.contains("BoneChain"))
			for (const auto& b : j["BoneChain"])
				boneNames.push_back(b.get<string>());
		if (boneNames.empty()) continue;

		// 3. 등록
		_uint iGoal = Register_Target(strName, eSolver, boneNames);

		// 4. Register_Goal이 안 담는 나머지 채우기
		IK_TARGET& goal = m_Targets[iGoal];
		if (j.contains("PoleVector") && j["PoleVector"].size() == 3)
			goal.Chain.vPoleVector = XMVectorSet(j["PoleVector"][0], j["PoleVector"][1], j["PoleVector"][2], 0.f);
		goal.Chain.fPosWeight = j.value("PosWeight", 1.f);
		goal.Chain.fRotWeight = j.value("RotWeight", 1.f);
	}
}


_uint CIKComponent::Register_Target(const _string& strName, EIKSOLVER_TYPE eSolver, const vector<_string>& BoneNames)
{
	// 1. 중복 방지.
	auto it = m_TargetHandles.find(strName);
	if (it != m_TargetHandles.end())
		return it->second;

	// 2. 본 이름을 통한 인덱스 찾기.
	IK_TARGET target{};
	target.strName = strName;
	target.eSolver = eSolver;

	for (auto& name : BoneNames)
	{
		_uint iBoneIndex = Find_BoneIndex(name.c_str());
		ASSERT_CRASH(iBoneIndex != UINT_MAX);
		target.Chain.BoneChain.push_back(iBoneIndex);
	}

	if (eSolver == EIKSOLVER_TYPE::TWO_BONE && target.Chain.BoneChain.size() >= 3)
	{
		const vector<CBone*>& Bones = m_pModelCom->Get_Bones();
		_uint iR = target.Chain.BoneChain[0];
		_uint iM = target.Chain.BoneChain[1];
		_uint iE = target.Chain.BoneChain[2];
		// 부모 자식 관계가 아닌 Bone인경우 Crash
		ASSERT_CRASH(Bones[iM]->Get_ParentIndex() == static_cast<_int>(iR));
		ASSERT_CRASH(Bones[iE]->Get_ParentIndex() == static_cast<_int>(iM));
	}

	// 3. push한 위치가 handle값
	_uint iGoal = static_cast<_uint>(m_Targets.size());
	m_Targets.push_back(target);

	// 4. Map 등록
	m_TargetHandles.emplace(strName, iGoal);

	return iGoal;
}


// Solver 생성.
HRESULT CIKComponent::Initialize_Prototype()
{
	m_Solvers.resize(ENUM_CLASS(EIKSOLVER_TYPE::END));
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::TWO_BONE)] = CIKSolver_TwoBone::Create(m_pDevice, m_pContext);
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::CCD)]		= CIKSolver_CCD::Create(m_pDevice, m_pContext);
	m_Solvers[ENUM_CLASS(EIKSOLVER_TYPE::FABRIK)]	= CIKSolver_Fabrik::Create(m_pDevice, m_pContext);
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

void CIKComponent::Execute(_float fTimeDelta)
{
	_uint iMinBone = UINT_MAX;

	for (auto& target : m_Targets)
	{
		if (!target.isEnable || !target.isTargetSet)
			continue;

		if (target.fTargetWeight <= 0.f && target.fCurWeight <= 0.f)
		{
			target.isEnable = false;
			continue;
		}

		// Ease Out 도입.
		_float fAlpha = 1.f - expf(-target.fBlendSpeed * fTimeDelta);
		target.fCurWeight += (target.fTargetWeight - target.fCurWeight) * fAlpha;

		if (fabsf(target.fTargetWeight - target.fCurWeight) < 1e-3f)
			target.fCurWeight = target.fTargetWeight;

		IK_SOLVE_CONTEXT Context{};
		Context.pBones = &m_pModelCom->Get_Bones();
		Context.pTarget = &target;
		Context.fTimeDelta = fTimeDelta;

		IK_RESULT tResult = m_Solvers[ENUM_CLASS(target.eSolver)]->Solve(Context);

		if (tResult.isSolved && !target.Chain.BoneChain.empty())
			iMinBone = min(iMinBone, target.Chain.BoneChain[0]);
	}

	// Solver가 동작하고 난 뒤 FK 수행. => 회전 전파.
	if (iMinBone != UINT_MAX)
		m_pModelCom->Update_BoneMatrix_Map(iMinBone);
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

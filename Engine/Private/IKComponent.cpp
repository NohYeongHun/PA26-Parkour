#include "EnginePch.h"
#include "IKComponent.h"
#include "Model.h"
#include "Transform.h"
#include "IKSolver_CCD.h"
#include "IKSolver_Fabrik.h"
#include "IKSolver_TwoBone.h"
#include "Bone.h"
#include "GameInstance.h"

CIKComponent::CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CIKComponent::CIKComponent(const CIKComponent& Prototype)
	: CComponent(Prototype)
	, m_Solvers { Prototype.m_Solvers }
{
	for (auto& pSolver : m_Solvers)
	{
		Safe_AddRef(pSolver);
		pSolver->Set_Owner(this);
	}
		
}

void CIKComponent::Update_ForwardKinematics(_uint iRootIdx)
{
	m_pModelCom->Update_BoneMatrix_Map(iRootIdx);
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
	Target.eAlignMode = EALIGN_MODE::NONE;   // 기본값 (Set_AlignMode로 지정)
	Target.Chain.fPosWeight = fPosWeight;
	Target.Chain.fRotWeight = fRotWeight;
	Target.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
	Target.fTargetWeight = 1.f;

}

void CIKComponent::Set_AlignMode(const _string& strTarget, EALIGN_MODE eAlignMode)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end()) return;
	m_Targets[it->second].eAlignMode = eAlignMode;
}

void CIKComponent::End_Target(const _string& strTarget, _float fBlendSec)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end()) return;
	IK_TARGET& target = m_Targets[it->second];

	target.fTargetWeight = 0.f;
	target.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
}

_bool CIKComponent::Get_TargetEndWorldPos(const _string& strTarget, _vector& vOutWorld)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end())
		return false;

	const IK_TARGET& target = m_Targets[it->second];
	if (target.Chain.BoneChain.empty())
		return false;

	_uint iEnd = target.Chain.BoneChain.back();
	const vector<CBone*>& Bones = m_pModelCom->Get_Bones();
	_vector vModelPos = XMLoadFloat4x4(Bones[iEnd]->Get_CombinedTransformationMatrix()).r[3];
	vOutWorld = XMVector3TransformCoord(vModelPos, m_matModelToWorld);

	return true;
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

	// 모델 스페이스 변환
	target.Chain.vTargetPos = vModelPos;
	target.vCurTargetPos = vModelPos;
	target.vTargetNormal = vModelNormal;
	target.isTargetSet = true;
}


// LeftArmTwoBone, RightArmTwoBone 등 특정 본 체인을 게임 시작 시 json으로 파싱해둡니다.
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
		_uint iTarget = Register_Target(strName, eSolver, boneNames);

		// 4. Register_Goal이 안 담는 나머지 채우기
		IK_TARGET& Target = m_Targets[iTarget];
		if (j.contains("PoleVector") && j["PoleVector"].size() == 3)
			Target.Chain.vPoleVector = XMVectorSet(j["PoleVector"][0], j["PoleVector"][1], j["PoleVector"][2], 0.f);
		if (j.contains("SoleAxis") && j["SoleAxis"].size() == 3)
			Target.Chain.vSoleAxis = XMVectorSet(j["SoleAxis"][0], j["SoleAxis"][1], j["SoleAxis"][2], 0.f);
		Target.Chain.fPosWeight = j.value("PosWeight", 1.f);
		Target.Chain.fRotWeight = j.value("RotWeight", 1.f);
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
	target.Chain.vSoleAxis = XMVectorSet(0.f, -1.f, 0.f, 0.f); // 기본값.

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
	_uint iTarget = static_cast<_uint>(m_Targets.size());
	m_Targets.push_back(target);

	// 4. Map 등록
	m_TargetHandles.emplace(strName, iTarget);

	return iTarget;
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

		if (target.fCurWeight < target.fTargetWeight)
			target.fCurWeight = min(target.fCurWeight + target.fBlendSpeed * fTimeDelta, target.fTargetWeight);
		else if (target.fCurWeight > target.fTargetWeight)
			target.fCurWeight = max(target.fCurWeight - target.fBlendSpeed * fTimeDelta, target.fTargetWeight);

		IK_SOLVE_CONTEXT Context{};
		Context.pBones = &m_pModelCom->Get_Bones();
		Context.pTarget = &target;
		Context.fTimeDelta = fTimeDelta;
		Context.matModelToWorld = m_matModelToWorld;

		IK_RESULT tResult = m_Solvers[ENUM_CLASS(target.eSolver)]->Solve(Context);
	}
}

#ifdef _DEBUG
void CIKComponent::Debug_DrawActiveIK(_fmatrix WorldMatrix, const JPH::Color& ChainColor, const JPH::Color& TargetColor)
{
	CGameInstance* pGI = CGameInstance::GetInstance();
	const vector<CBone*>& Bones = m_pModelCom->Get_Bones();

	for (const IK_TARGET& target : m_Targets)
	{
		if (!target.isEnable || !target.isTargetSet)
			continue;

		const vector<_uint>& Chain = target.Chain.BoneChain;
		for (size_t k = 1; k < Chain.size(); ++k)
		{
			_matrix Prev = XMLoadFloat4x4(Bones[Chain[k - 1]]->Get_CombinedTransformationMatrix()) * WorldMatrix;
			_matrix Cur  = XMLoadFloat4x4(Bones[Chain[k]]->Get_CombinedTransformationMatrix()) * WorldMatrix;
			pGI->Add_DebugLine(Prev.r[3], Cur.r[3], ChainColor);
		}

		_vector vTargetWorld = XMVector3TransformCoord(target.vCurTargetPos, WorldMatrix);
		const _float fCross = 0.05f;
		_vector vX = XMVectorSet(fCross, 0.f, 0.f, 0.f);
		_vector vY = XMVectorSet(0.f, fCross, 0.f, 0.f);
		_vector vZ = XMVectorSet(0.f, 0.f, fCross, 0.f);
		pGI->Add_DebugLine(XMVectorSubtract(vTargetWorld, vX), XMVectorAdd(vTargetWorld, vX), TargetColor);
		pGI->Add_DebugLine(XMVectorSubtract(vTargetWorld, vY), XMVectorAdd(vTargetWorld, vY), TargetColor);
		pGI->Add_DebugLine(XMVectorSubtract(vTargetWorld, vZ), XMVectorAdd(vTargetWorld, vZ), TargetColor);
	}
}
#endif

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

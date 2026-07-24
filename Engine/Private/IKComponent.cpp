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
		Safe_AddRef(pSolver);
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


	Target.Runtime.isEnable = true;
	Target.Runtime.isTargetSet = false;
	Target.Runtime.bHasPole = false;
	Target.Runtime.eMode = eMode;
	Target.Chain.fPosWeight = fPosWeight;
	Target.Chain.fRotWeight = fRotWeight;
	Target.Runtime.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
	Target.Runtime.fTargetWeight = 1.f;

}

void CIKComponent::Set_TargetAlpha(const _string& strTarget, _float fAlpha)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end())
		return;

	IK_TARGET& target = m_Targets[it->second];
	target.Runtime.fTargetWeight = fAlpha;
	target.Runtime.fCurWeight = fAlpha;
}


void CIKComponent::End_Target(const _string& strTarget, _float fBlendSec)
{
	auto it = m_TargetHandles.find(strTarget);
	if (it == m_TargetHandles.end()) return;
	IK_TARGET& target = m_Targets[it->second];

	target.Runtime.fTargetWeight = 0.f;
	target.Runtime.fBlendSpeed = (fBlendSec > 0.f) ? 1.f / fBlendSec : FLT_MAX;
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
	_vector vModelPos = XMVector3TransformCoord(vWorldPos, m_matWorldToModel); // 모델 스페이스 변환
	_vector vModelNormal = XMVector3TransformNormal(vNormal, m_matWorldToModel); // 모델 스페이스 변환

	// 모델 스페이스 변환
	target.Chain.vTargetPos = vModelPos;
	target.Runtime.vCurTargetPos = vModelPos;
	target.Runtime.vTargetNormal = vModelNormal;
	target.Runtime.isTargetSet = true;
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

		// 2. 본 체인 (root->end)
		vector<_string> boneNames;
		if (j.contains("BoneChain"))
			for (const auto& b : j["BoneChain"])
				boneNames.push_back(b.get<string>());
		if (boneNames.empty()) continue;

		// 3. 등록
		_uint iTarget = Register_Target(strName, eSolver, boneNames);

		// 4. Register_Goal이 안 담는 나머지 채우기
		IK_TARGET& Target = m_Targets[iTarget];
		Target.Chain.fPosWeight = j.value("PosWeight", 1.f);
		Target.Chain.fRotWeight = j.value("RotWeight", 1.f);

		// 5. Parse
		Parse_SolverOptions(j, Target);

		_uint iMax = 0;
		for (const IK_TARGET& t : m_Targets)
			iMax = max(iMax, static_cast<_uint>(t.Chain.BoneChain.size()));

		m_Workspace.Positions.reserve(iMax);
		m_Workspace.Lengths.reserve(iMax);

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

	const vector<CBone*>& Bones = m_pModelCom->Get_Bones();
	const vector<_uint>& ChainIdx = target.Chain.BoneChain;

	// 전 솔버 공통 불변식 (개수와 무관함)
	ASSERT_CRASH(ChainIdx.size() >= 3);
	ASSERT_CRASH(Bones[ChainIdx[0]]->Get_ParentIndex() >= 0); // Root 뼈는 부모가 필수
	for (size_t k = 1; k < ChainIdx.size(); ++k)
		ASSERT_CRASH(Bones[ChainIdx[k]]->Get_ParentIndex() == static_cast<_int>(ChainIdx[k - 1]));

	// TwoBone 개수 조건
	if (eSolver == EIKSOLVER_TYPE::TWO_BONE)
		ASSERT_CRASH(ChainIdx.size() == 3);

	// 3. 끝 본의 자손 인덱스 캐시 — 토폴로지는 런타임 불변이므로 등록 시 1회만 계산.
	//    (침투 측정이 매 프레임 전체 본 × 조상 탐색하던 것을 목록 순회로 대체)
	const _uint iEnd = ChainIdx.back();
	for (_uint i = iEnd + 1; i < static_cast<_uint>(Bones.size()); ++i)
	{
		_int iParent = Bones[i]->Get_ParentIndex();
		while (iParent >= 0 && static_cast<_uint>(iParent) != iEnd)
			iParent = Bones[static_cast<_uint>(iParent)]->Get_ParentIndex();
		if (iParent >= 0)
			target.Chain.EndSubtree.push_back(i);
	}

	// 4. push한 위치가 handle값
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
		if (!target.Runtime.isEnable || !target.Runtime.isTargetSet)
			continue;

		if (target.Runtime.fTargetWeight <= 0.f && target.Runtime.fCurWeight <= 0.f)
		{
			target.Runtime.isEnable = false;
			continue;
		}

		if (target.Runtime.fCurWeight < target.Runtime.fTargetWeight)
			target.Runtime.fCurWeight = min(target.Runtime.fCurWeight + target.Runtime.fBlendSpeed * fTimeDelta
				, target.Runtime.fTargetWeight);
		else if (target.Runtime.fCurWeight > target.Runtime.fTargetWeight)
			target.Runtime.fCurWeight = max(target.Runtime.fCurWeight - target.Runtime.fBlendSpeed * fTimeDelta, target.Runtime.fTargetWeight);

		IK_SOLVE_CONTEXT Context{};
		Context.pBones = &m_pModelCom->Get_Bones();
		Context.pTarget = &target;
		Context.pOwner = this;
		Context.pWorkspace = &m_Workspace;
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
		if (!target.Runtime.isEnable || !target.Runtime.isTargetSet)
			continue;

		const vector<_uint>& Chain = target.Chain.BoneChain;
		for (size_t k = 1; k < Chain.size(); ++k)
		{
			_matrix Prev = XMLoadFloat4x4(Bones[Chain[k - 1]]->Get_CombinedTransformationMatrix()) * WorldMatrix;
			_matrix Cur  = XMLoadFloat4x4(Bones[Chain[k]]->Get_CombinedTransformationMatrix()) * WorldMatrix;
			pGI->Add_DebugLine(Prev.r[3], Cur.r[3], ChainColor);
		}

		_vector vTargetWorld = XMVector3TransformCoord(target.Runtime.vCurTargetPos, WorldMatrix);
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

// 솔버별 파라미터는 "Option" 오브젝트 하위에 저작합니다. (미저작 시 구조체 기본값 유지)
void CIKComponent::Parse_SolverOptions(const json& j, IK_TARGET& Target)
{
	if (!j.contains("Option") || !j["Option"].is_object())
		return;

	const json& Option = j["Option"];

	switch (Target.eSolver)
	{
	case EIKSOLVER_TYPE::TWO_BONE:

		if (Option.contains("PoleVector") && Option["PoleVector"].size() == 3)
			Target.OptTwoBone.vPoleVector = XMVectorSet(Option["PoleVector"][0], Option["PoleVector"][1], Option["PoleVector"][2], 0.f);
		break;

	case EIKSOLVER_TYPE::FABRIK:
		if (Option.contains("PoleVector") && Option["PoleVector"].size() == 3)
			Target.OptFabrik.vPoleVector = XMVectorSet(Option["PoleVector"][0], Option["PoleVector"][1], Option["PoleVector"][2], 0.f);
		Target.OptFabrik.fTolerance = Option.value("Tolerance", 0.001f);
		Target.OptFabrik.iMaxIterations = Option.value("MaxIterations", 10u);
		break;

	case EIKSOLVER_TYPE::CCD:
		Target.OptCCD.fTolerance = Option.value("Tolerance", 0.001f);
		Target.OptCCD.iMaxIterations = Option.value("MaxIterations", 10u);
		break;
	}
	
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

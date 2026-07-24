#include "EnginePch.h"
#include "IKSolver_TwoBone.h"
#include "GameObject.h"
#include "IKComponent.h"
#include "Bone.h"
#include "Engine_Profile.h"
#include "GameInstance.h"   // 디버그 드로우용

CIKSolver_TwoBone::CIKSolver_TwoBone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSolver_TwoBone::CIKSolver_TwoBone(const CIKSolver_TwoBone& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSolver_TwoBone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSolver_TwoBone::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSolver_TwoBone::Render()
{
	return S_OK;
}

IK_RESULT CIKSolver_TwoBone::Solve(const IK_SOLVE_CONTEXT& Context)
{

	IK_RESULT tResult = Update_InverseKinematics(Context);

	// Update FK

	return tResult;
}



IK_RESULT CIKSolver_TwoBone::Update_InverseKinematics(const IK_SOLVE_CONTEXT& Context)
{
	PROFILE_ZONE();
	IK_RESULT tResult{};
	tResult.isSolved = true;

	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;
	if (Chain.size() < 3)
		return tResult;
	if (Target.Runtime.fCurWeight <= 0.f)
		return tResult;

	_uint iRoot = Chain[0];
	_uint iMid = Chain[1];

	_vector vPlaneNormal = XMVector3Normalize(Target.Runtime.vTargetNormal);
	_vector vPlanePoint = Target.Runtime.vCurTargetPos;
	_vector vSolveTarget = vPlanePoint;
	if (Target.Runtime.eMode == EIKTARGET_MODE::POSITION_CLEARANCE)
	{
		PROFILE_ZONE_N("DeepestPeneration");
		// 1. 기존 Transformatrix 저장.
		_matrix matRootSave = XMLoadFloat4x4(Bones[iRoot]->Get_TransformationMatrix());
		_matrix matMidSave = XMLoadFloat4x4(Bones[iMid]->Get_TransformationMatrix());

		// 2. 타겟지점으로의 첫번째 IK
		Solve_TwoBonePosition(Context, vPlanePoint, 1.f);
		Context.pOwner->Update_ForwardKinematics(iRoot);

		_int iDeep = -1;
		// 3. Ray에 타겟된 지점과의 침투 깊이를 구합니다.
		_float fPen = Measure_DeepestPenetration(Bones, Target.Chain.EndSubtree, vPlanePoint, vPlaneNormal, iDeep);

		Bones[iRoot]->Set_TransformationMatrix(matRootSave);
		Bones[iMid]->Set_TransformationMatrix(matMidSave);
		Context.pOwner->Update_ForwardKinematics(iRoot);

		// 3. 침투 깊이를 통한 새로운 Target지점을 구합니다.
		if (fPen < 0.f && iDeep >= 0)
		{
			const _float fSkin = 0.02f;
			vSolveTarget = vPlanePoint + (-fPen + fSkin) * vPlaneNormal;
		}
	}
	// 4. 보정된 타겟지점으로 다시 IK를 수행합니다.
	Solve_TwoBonePosition(Context, vSolveTarget, Target.Runtime.fCurWeight);
	Context.pOwner->Update_ForwardKinematics(iRoot);
	return tResult;
}

void CIKSolver_TwoBone::Solve_TwoBonePosition(const IK_SOLVE_CONTEXT& Context, _fvector vTargetPos, _float fWeight)
{
	PROFILE_ZONE();
	const _float fEPSILON = 1e-6f;
	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;

	_uint iRoot = Chain[0];
	_uint iMid = Chain[1];
	_uint iEnd = Chain[2];

	_vector vRootPos = XMLoadFloat4x4(Bones[iRoot]->Get_CombinedTransformationMatrix()).r[3];
	_vector vMidPos = XMLoadFloat4x4(Bones[iMid]->Get_CombinedTransformationMatrix()).r[3];
	_vector vEndPos = XMLoadFloat4x4(Bones[iEnd]->Get_CombinedTransformationMatrix()).r[3];
	_vector vJointTargetPos = vTargetPos;

	_vector vUpper = vMidPos - vRootPos;
	_vector vLower = vEndPos - vMidPos;
	_vector vJointTarget = vJointTargetPos - vRootPos;

	_float l1 = XMVectorGetX(XMVector3Length(vUpper));
	_float l2 = XMVectorGetX(XMVector3Length(vLower));
	_float d = XMVectorGetX(XMVector3Length(vJointTarget));

	_float cosAngle = std::clamp((l1 * l1 + d * d - l2 * l2) / ((2 * l1) * d), -1.f, 1.f);
	_float angle = acosf(cosAngle);

	_vector vForward = XMVector3Normalize(vJointTarget);
	_vector vBendDir;
	if (d < 0.1f)
	{
		vForward = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		vBendDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	}
	else
	{
		_vector vPole = CIKSolver::TwoBoneMakePoleVector(vRootPos, vMidPos, vEndPos);
		vBendDir = vPole - XMVector3Dot(vPole, vForward) * vForward;
		if (XMVectorGetX(XMVector3LengthSq(vBendDir)) < fEPSILON)
		{
			vBendDir = XMVector3Cross(vForward, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			if (XMVectorGetX(XMVector3LengthSq(vBendDir)) < fEPSILON)
				vBendDir = XMVector3Cross(vForward, XMVectorSet(1.f, 0.f, 0.f, 0.f));
		}
		vBendDir = XMVector3Normalize(vBendDir);
	}

	_float  projDist = l1 * cosAngle;
	_float  jointLineDist = l1 * sinf(angle);
	_vector vSolvedMidPos = vRootPos + projDist * vForward + jointLineDist * vBendDir;
	_vector vSolvedEndPos = vJointTargetPos;

	_vector vCurUpperDir = XMVector3Normalize(vMidPos - vRootPos); // Root -> Mid
	_vector vTgtUpperDir = XMVector3Normalize(vSolvedMidPos - vRootPos); // Root -> NewMid
	_vector qRootDelta = QuatFromTo(vCurUpperDir, vTgtUpperDir);
	qRootDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qRootDelta, fWeight);

	// 새로운 Mid 위치는 기존 벡터를 회전하고 길이를 곱해서 길이는 유지하되 회전된 위치로.
	_vector vNewMidPos = vRootPos + XMVector3Rotate(vCurUpperDir, qRootDelta) * l1;
	_vector vTgtLowerDir = XMVector3Normalize(vSolvedEndPos - vNewMidPos);
	_vector vCurLowerDir = XMVector3Rotate(XMVector3Normalize(vEndPos - vMidPos), qRootDelta);
	_vector qMidDelta = QuatFromTo(vCurLowerDir, vTgtLowerDir);
	qMidDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qMidDelta, fWeight);

	auto PivotRot = [](_fvector qDelta, _fvector vPivot) -> _matrix {
		return XMMatrixTranslationFromVector(-vPivot)
			* XMMatrixRotationQuaternion(qDelta)
			* XMMatrixTranslationFromVector(vPivot);
	};

	_int iRootParent = Bones[iRoot]->Get_ParentIndex();
	_matrix matRootParent = XMLoadFloat4x4(Bones[iRootParent]->Get_CombinedTransformationMatrix());
	_matrix mRootPivot = PivotRot(qRootDelta, vRootPos);
	_matrix matRootComb = XMLoadFloat4x4(Bones[iRoot]->Get_CombinedTransformationMatrix()) * mRootPivot;

	// 부모 행렬의 역행렬을 곱해줌으로써 로컬행렬 상태를 갱신합니다.
	Bones[iRoot]->Set_TransformationMatrix(matRootComb * XMMatrixInverse(nullptr, matRootParent));

	_matrix mMidPivot = PivotRot(qMidDelta, vNewMidPos);
	_matrix matMidComb = (XMLoadFloat4x4(Bones[iMid]->Get_CombinedTransformationMatrix()) * mRootPivot) * mMidPivot;
	// 부모 행렬의 역행렬을 곱해줌으로써 로컬행렬 상태를 갱신합니다.
	Bones[iMid]->Set_TransformationMatrix(matMidComb * XMMatrixInverse(nullptr, matRootComb));
}

CIKSolver_TwoBone* CIKSolver_TwoBone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSolver_TwoBone* pInstance = new CIKSolver_TwoBone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_TwoBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSolver_TwoBone::Clone(void* pArg)
{
	CIKSolver_TwoBone* pClone = new CIKSolver_TwoBone(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_TwoBone (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSolver_TwoBone::Free()
{
	__super::Free();
}

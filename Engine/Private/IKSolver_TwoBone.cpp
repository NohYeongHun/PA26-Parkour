#include "EnginePch.h"
#include "IKSolver_TwoBone.h"
#include "GameObject.h"
#include "Bone.h"
#include "Engine_Profile.h"

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
	PROFILE_ZONE();
	IK_RESULT tResult{};

	const _float fEPSILON = 1e-6f;
	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;
	if (Chain.size() < 3)
		return tResult;

	_float fWeight = Target.fCurWeight; // 블렌딩할 가중치.
	if (fWeight <= 0.f)
		return tResult;

	_uint iRoot = Chain[0];
	_uint iMid = Chain[1];
	_uint iEnd = Chain[2];

	_vector vNormal = Target.vTargetNormal;
	// 0. 도달할 수 있는 최소 거리는 뼈 길이 차이의 절대값 |l1 - l2| >= d
	//    도달할 수 있는 최대 거리는 뼈 길이의 합 |l1 + l2 | < d
	//	  | l1 - l2 | <= d < | l1 + l2 |
	//    못 푸는 해라 할지라도 팔을 쭉 뻗는다던가 하는 식으로 (최대한 근사치) 구현

	// 1. 현재 Position 읽기.
	_vector vRootPos = XMLoadFloat4x4(Bones[iRoot]->Get_CombinedTransformationMatrix()).r[3];
	_vector vMidPos = XMLoadFloat4x4(Bones[iMid]->Get_CombinedTransformationMatrix()).r[3];
	_vector vEndPos = XMLoadFloat4x4(Bones[iEnd]->Get_CombinedTransformationMatrix()).r[3];
	_vector vJointTargetPos = Target.vCurTargetPos;

	_vector vUpper = vMidPos - vRootPos;				// Root -> Mid
	_vector vLower = vEndPos - vMidPos;					// Mid -> End
	_vector vJointTarget = vJointTargetPos - vRootPos;	// Root -> Target

	_float l1 = XMVectorGetX(XMVector3Length(vUpper));	 
	_float l2 = XMVectorGetX(XMVector3Length(vLower));	
	_float d = XMVectorGetX(XMVector3Length(vJointTarget));
	
	// 2. 목표 값으로의 회전을 위한 사잇각 구하기.
	// Analytic Two Bone IK => 무한한 평면에서도 모든 각도가 사용이 가능함
	// 그래서 무한한 평면 해 중에서 평면을 고르는 과정이 중요하다.
	_float cosTheta1 = std::clamp((l1 * l1 + d * d - l2 * l2) / ((2 * l1) * d), -1.f, 1.f);
	_float theta1 = acosf(cosTheta1);
	_float cosTheta2 = std::clamp((l1 * l1 + l2 * l2 - d * d) / (2 * l1 * l2), -1.f, 1.f);
	_float theta2 = acosf(cosTheta2);

	// 3. 굽힘 평면 구하기
	// => 방향 벡터 구하기.
	_vector vForward = XMVector3Normalize(vJointTarget);
	_vector vJointPlaneNormal, vJointBendDir;
	if (d < 0.1f) // 길이가 너무 짧다면?
	{
		vJointPlaneNormal = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		vJointBendDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	}
	else
	{
		vJointBendDir = CIKSolver::TwoBoneMakePoleVector(vRootPos, vMidPos, vEndPos);
		// 회전 축.
		vJointPlaneNormal = XMVector3Cross(vForward, vJointBendDir);

		if (XMVectorGetX(XMVector3LengthSq(vJointPlaneNormal)) < fEPSILON)
			vJointPlaneNormal = XMVectorSet(0.f, 1.f, 0.f, 0.f); // 법선 길이가 너무 작다면?
		else
			vJointPlaneNormal = XMVector3Normalize(vJointPlaneNormal);
	}

	// 4. 굽힘 평면을 이용하여 회전

	// Root 회전
	_vector vCurUpperDir	= XMVector3Normalize(vMidPos - vRootPos); // Root -> Mid
	_vector qRootTilt		= XMQuaternionRotationAxis(vJointPlaneNormal, theta1); // 특정 회전 쿼터니언
	_vector vDesiredUpper	= XMVector3Rotate(vForward, qRootTilt);                // Root->Target 방향 벡터를 쿼터니언으로 회전시킴.
	_vector qRootDelta		= QuatFromTo(vCurUpperDir, vDesiredUpper);             // Root에 줄 모델스페이스 델타 => vRoot -> vMid 회전을 수정하는 변환 벡터
	qRootDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qRootDelta, fWeight);   // 현재 가중치에 맞게 구면 선형보간

	// Mid 회전 (각도 기반: theta2 사용)
	vDesiredUpper = XMVector3Rotate(vCurUpperDir, qRootDelta); // weight 반영된 실제 상단 방향
	_vector vNewMidPos = vRootPos + vDesiredUpper * l1;        // 피벗 회전에 여전히 필요

	// 하단 뼈 = 상단 뼈를 (π - theta2)만큼 꺾은 방향
	// 부호(-)는 theta1 기울인 방향과 반대로 접혀야 타겟으로 되돌아오기 때문
	_float   bendAngle = -(XM_PI - theta2);
	_vector  qMidBend = XMQuaternionRotationAxis(vJointPlaneNormal, bendAngle);
	_vector  vDesiredLower = XMVector3Rotate(vDesiredUpper, qMidBend);

	_vector vCurLowerDir = XMVector3Rotate(XMVector3Normalize(vEndPos - vMidPos), qRootDelta);
	_vector qMidDelta = QuatFromTo(vCurLowerDir, vDesiredLower);
	qMidDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qMidDelta, fWeight); 

	
	/*
		1. Pivot의 점을 원점으로 돌리기
		2. 변화량으로 회전
		3. Pivot으로 다시 변경
	*/

	auto PivotRot = [](_fvector qDelta, _fvector vPivot) -> _matrix {
		return XMMatrixTranslationFromVector(-vPivot)
			* XMMatrixRotationQuaternion(qDelta)
			* XMMatrixTranslationFromVector(vPivot);
	};

	_int iRootParent = Bones[iRoot]->Get_ParentIndex();
	_matrix matRootParent = XMLoadFloat4x4(Bones[iRootParent]->Get_CombinedTransformationMatrix());
	_matrix mRootPivot = PivotRot(qRootDelta, vRootPos);
	_matrix matRootComb = XMLoadFloat4x4(Bones[iRoot]->Get_CombinedTransformationMatrix()) * mRootPivot;

	// 현재 계산한 Matrix는 Model Space이므로 뼈의 Local Space로 돌려줍니다.
	Bones[iRoot]->Set_TransformationMatrix(matRootComb * XMMatrixInverse(nullptr, matRootParent));

	_matrix mMidPivot = PivotRot(qMidDelta, vNewMidPos);
	_matrix matMidComb = (XMLoadFloat4x4(Bones[iMid]->Get_CombinedTransformationMatrix()) * mRootPivot) * mMidPivot;
	// 현재 계산한 Matrix는 Model Space이므로 뼈의 Local Space로 돌려줍니다.
	Bones[iMid]->Set_TransformationMatrix(matMidComb * XMMatrixInverse(nullptr, matRootComb));

	tResult.isSolved = true;
	return tResult;
}



const _char* CIKSolver_TwoBone::Get_Name() const
{
	return nullptr;
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

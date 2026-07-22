#include "EnginePch.h"
#include "IKSolver_TwoBone.h"
#include "GameObject.h"
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
	PROFILE_ZONE();
	IK_RESULT tResult = Update_InverseKinematics(Context);

	// Update FK
	if (tResult.isSolved && !Context.pTarget->Chain.BoneChain.empty())
	{
		m_pOwner->Update_ForwardKinematics(Context.pTarget->Chain.BoneChain[0]);
	}
	
	return tResult;
}



const _char* CIKSolver_TwoBone::Get_Name() const
{
	return nullptr;
}

IK_RESULT CIKSolver_TwoBone::Update_InverseKinematics(const IK_SOLVE_CONTEXT& Context)
{
	IK_RESULT tResult{};
	tResult.isSolved = true;

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
	//    못 푸는 해라 할지라도 최대한 근사치 구현 => 팔을 쭉 뻗는다던가 하는 식으로

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

	// 2. 코사인 법칙으로 상단 뼈 각도 계산
	_float cosAngle = std::clamp((l1 * l1 + d * d - l2 * l2) / ((2 * l1) * d), -1.f, 1.f);
	_float angle = acosf(cosAngle);

	// 3. 굽힘 평면 기저 구하기: forward(Root->Target) + forward에 직교하는 bend 방향
	_vector vForward = XMVector3Normalize(vJointTarget);
	_vector vBendDir;
	if (d < 0.1f) // 타겟이 Root에 너무 붙어 방향이 불안정한 경우
	{
		vForward = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		vBendDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	}
	else
	{
		// 폴 벡터를 forward에 직교하도록 정사영
		_vector vPole = CIKSolver::TwoBoneMakePoleVector(vRootPos, vMidPos, vEndPos);
		vBendDir = vPole - XMVector3Dot(vPole, vForward) * vForward;
		if (XMVectorGetX(XMVector3LengthSq(vBendDir)) < fEPSILON) // 굽힘 정도가 거의 없고 평행하다면?
		{
			// 폴이 forward와 거의 평행 => forward에 직교하는 임의 축 선택
			vBendDir = XMVector3Cross(vForward, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			if (XMVectorGetX(XMVector3LengthSq(vBendDir)) < fEPSILON)
				vBendDir = XMVector3Cross(vForward, XMVectorSet(1.f, 0.f, 0.f, 0.f));
		}
		vBendDir = XMVector3Normalize(vBendDir);
	}

	// 4. 위치 삼각형을 풀어 관절/엔드의 목표 좌표 확정.
	//    상단 뼈를 forward 축 성분 + 평면 수직 성분으로 분해.
	_float  projDist = l1 * cosAngle;          // forward 축 방향 성분 (부호 포함)
	_float  jointLineDist = l1 * sinf(angle);       // 평면 수직 성분
	_vector vSolvedMidPos = vRootPos + projDist * vForward + jointLineDist * vBendDir; // Mid를 회전할 구간.
	_vector vSolvedEndPos = vJointTargetPos;        // 엔드의 목표 = 이펙터(타겟)

	// 확정된 좌표를 향해 각 뼈를 회전시켜 실제 스켈레톤을 갱신.

	// Root: 상단 뼈를 (Root -> 목표 관절 위치) 방향으로 회전
	_vector vCurUpperDir = XMVector3Normalize(vMidPos - vRootPos);
	_vector vTgtUpperDir = XMVector3Normalize(vSolvedMidPos - vRootPos);
	_vector qRootDelta = QuatFromTo(vCurUpperDir, vTgtUpperDir);
	qRootDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qRootDelta, fWeight); // 가중치 블렌딩

	// Mid: Root 회전이 전파된 뒤, 하단 뼈를 (회전된 관절 -> 목표 엔드) 방향으로 회전
	_vector vNewMidPos = vRootPos + XMVector3Rotate(vCurUpperDir, qRootDelta) * l1; // 가중치 반영된 실제 관절 위치
	_vector vTgtLowerDir = XMVector3Normalize(vSolvedEndPos - vNewMidPos);
	_vector vCurLowerDir = XMVector3Rotate(XMVector3Normalize(vEndPos - vMidPos), qRootDelta); // Root 회전 전파 반영
	_vector qMidDelta = QuatFromTo(vCurLowerDir, vTgtLowerDir);
	qMidDelta = XMQuaternionSlerp(XMQuaternionIdentity(), qMidDelta, fWeight);


	/*
		1. Pivot의 점을 원점으로 돌리기
		2. qDelta 변화량으로 회전
		3. Pivot으로 다시 변경
		=> 이러한 변환 행렬을 생성.
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

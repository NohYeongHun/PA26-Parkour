#include "EnginePch.h"
#include "IKSolver_TwoBone.h"
#include "GameObject.h"
#include "Bone.h"

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
	IK_RESULT tResult{};

	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;
	if (Chain.size() < 3)
		return tResult;

	_float fWeight = Target.fCurWeight;
	if (fWeight <= 0.f)
		return tResult;

	_uint iRoot = Chain[0];
	_uint iMid = Chain[1];
	_uint iEnd = Chain[2];

	_vector vNormal = Target.vTargetNormal;
	// 0. 도달할 수 있는 최소 거리는 뼈 길이 차이의 절대값
	//    도달할 수 있는 최대 거리는 뼈 길이의 합
	//    게임 엔진에서는 못 푸는 해라 할지라도 팔을 쭉 뻗는다던가 하는 식으로 (최대한 근사치) 구현

	// 1. 현재 Position 읽기.
	_vector vRootPos = XMLoadFloat4x4(Bones[iRoot]->Get_CombinedTransformationMatrix()).r[3];
	_vector vMidPos = XMLoadFloat4x4(Bones[iMid]->Get_CombinedTransformationMatrix()).r[3];
	_vector vEndPos = XMLoadFloat4x4(Bones[iEnd]->Get_CombinedTransformationMatrix()).r[3];
	_vector vTargetPos = Target.vCurTargetPos;

	_vector vUpper = vMidPos - vRootPos;
	_vector vLower = vEndPos - vMidPos;
	_vector vTarget = vRootPos - vTargetPos;

	_float l1 = XMVectorGetX(XMVector3Length(vUpper));	 // Root -> Mid
	_float l2 = XMVectorGetX(XMVector3Length(vLower));	 // Mid -> End
	_float d = XMVectorGetX(XMVector3Length(vTarget)); // Root -> Target
	
	// 2. 사잇각 구하기. => 목표 값으로의 회전
	_float cosTheta1 = std::clamp((l1 * l1 + d * d - l2 * l2) / ((2 * l1) * d), -1.f, 1.f);
	_float theta1 = acosf(cosTheta1);
	_float cosTheta2 = std::clamp((l1 * l1 + l2 * l2 - d * d) / (2 * l1 * l2), -1.f, 1.f);
	_float theta2 = acosf(cosTheta2);

	// 3. 회전축 구하기.
	_vector vTargetDir = XMVector3Normalize(vTargetPos - vRootPos);
	_vector vPole = XMVector3Normalize(XMVector3Cross(XMVector3Cross(vUpper, vLower), vUpper));

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

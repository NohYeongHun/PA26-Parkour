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


/*
	Two-Bone IK 계산에 필요한 것.
	Bone Chain(root joint / mid joint / end joint) 인덱스
	본 배열 (관절 위치 읽기, write-back)
	타깃 위치 (모델 공간)
	pole vector(굽힘 평면)
	가중치
	블렌드 계수
	모드(위치 / 회전)
	fTimeDelta;
*/
IK_RESULT CIKSolver_TwoBone::Solve(const IK_SOLVE_CONTEXT& Context)
{
	const vector<CBone*>* pBones = Context.pBones;
	const IK_TARGET* pTarget = Context.pTarget;
	pTarget->vCurTargetPos;

	return IK_RESULT();
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

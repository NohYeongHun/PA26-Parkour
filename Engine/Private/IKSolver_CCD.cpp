#include "EnginePch.h"
#include "Engine_Profile.h"
#include "IKSolver_CCD.h"
#include "GameObject.h"
#include "IKComponent.h"
#include "Bone.h"

CIKSolver_CCD::CIKSolver_CCD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSolver_CCD::CIKSolver_CCD(const CIKSolver_CCD& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSolver_CCD::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSolver_CCD::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSolver_CCD::Render()
{
	return S_OK;
}


IK_RESULT CIKSolver_CCD::Solve(const IK_SOLVE_CONTEXT& Context)
{
	PROFILE_ZONE(); // Release에서만 실행될 때 동작함.
	return Update_InverseKinematics(Context);
}

IK_RESULT CIKSolver_CCD::Update_InverseKinematics(const IK_SOLVE_CONTEXT& Context)
{
	IK_RESULT tResult{};
	tResult.isSolved = true;

	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;

	if (Chain.size() < 3)
		return tResult;
	if (Target.Runtime.fCurWeight <= 0.f)
		return tResult;

	if (!Gather_Chain(Context))
		return tResult;

	_float fErr = 0.f;
	_uint iIterations = Iterate_CCD(Context, Target.Runtime.vCurTargetPos, fErr);

	Apply_ChainPositions(Context, Target.Runtime.fCurWeight);
	Context.pOwner->Update_ForwardKinematics(Chain[0]);

	tResult.fPosError = fErr;
	tResult.iIterations = iIterations;
	return tResult;
}

_uint CIKSolver_CCD::Iterate_CCD(const IK_SOLVE_CONTEXT& Context, _fvector vTarget, _float& fErrOut)
{
	const _float fEPSILON = 1e-6f;
	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;
	const IK_OPTION_CCD& Opt = Context.pTarget->OptCCD;

	vector<_vector>& P = WS.Positions;
	const size_t iN = P.size();

	fErrOut = XMVectorGetX(XMVector3Length(P[iN - 1] - vTarget));

	_uint iIterDone = 0;
	for (_uint iIter = 0; iIter < Opt.iMaxIterations; ++iIter)
	{
		if (fErrOut < Opt.fTolerance)
			break;

		for (size_t i = iN - 1; i-- > 0; )
		{
			_vector vFrom = P[iN - 1] - P[i];
			_vector vTo = vTarget - P[i];

			if (XMVectorGetX(XMVector3LengthSq(vFrom)) < fEPSILON ||
				XMVectorGetX(XMVector3LengthSq(vTo)) < fEPSILON)
				continue;

			_vector qRot = QuatFromTo(XMVector3Normalize(vFrom), XMVector3Normalize(vTo));

			// 관절 i 하위 위치들을 P[i] 중심으로 회전
			for (size_t k = i + 1; k < iN; ++k)
				P[k] = P[i] + XMVector3Rotate(P[k] - P[i], qRot);
		}

		++iIterDone;
		fErrOut = XMVectorGetX(XMVector3Length(P[iN - 1] - vTarget));
	}

	return iIterDone;
}


CIKSolver_CCD* CIKSolver_CCD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSolver_CCD* pInstance = new CIKSolver_CCD(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_CCD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSolver_CCD::Clone(void* pArg)
{
	CIKSolver_CCD* pClone = new CIKSolver_CCD(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_CCD (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSolver_CCD::Free()
{
	__super::Free();
}

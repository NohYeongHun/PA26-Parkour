#include "EnginePch.h"
#include "Engine_Profile.h"
#include "IKSolver_Fabrik.h"
#include "GameObject.h"
#include "IKComponent.h"
#include "Bone.h"


CIKSolver_Fabrik::CIKSolver_Fabrik(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSolver_Fabrik::CIKSolver_Fabrik(const CIKSolver_Fabrik& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSolver_Fabrik::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSolver_Fabrik::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSolver_Fabrik::Render()
{
	return S_OK;
}

IK_RESULT CIKSolver_Fabrik::Solve(const IK_SOLVE_CONTEXT& Context)
{
	PROFILE_ZONE();
	IK_RESULT tResult = Update_InverseKinematics(Context);

	return tResult;
}

IK_RESULT CIKSolver_Fabrik::Update_InverseKinematics(const IK_SOLVE_CONTEXT& Context)
{
	IK_RESULT tResult{};
	tResult.isSolved = true;

	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;

	_uint iChainSize = static_cast<_uint>(Chain.size());
	if (iChainSize < 3)
		return tResult;
	if (Target.Runtime.fCurWeight <= 0.f)
		return tResult;

	_uint iRoot = Chain[0];

	_vector vPlaneNormal = XMVector3Normalize(Target.Runtime.vTargetNormal);
	_vector vPlanePoint = Target.Runtime.vCurTargetPos;
	_vector vSolveTarget = vPlanePoint;

	// 충돌 깊이만큼 보정합니다.
	if (Target.Runtime.eMode == EIKTARGET_MODE::POSITION_CLEARANCE)
	{
		vector<_float4x4> SavedLocals;
		SavedLocals.reserve(iChainSize - 1);
		for (_uint i = 0; i + 1 < iChainSize; ++i)
			SavedLocals.push_back(*Bones[Chain[i]]->Get_TransformationMatrix());

		Solve_FabrikPosition(Context, vPlanePoint, 1.f);
		Context.pOwner->Update_ForwardKinematics(iRoot);

		_int iDeep = -1;
		_float fPen = Measure_DeepestPenetration(Bones, Target.Chain.EndSubtree, vPlanePoint, vPlaneNormal, iDeep);

		for (_uint i = 0; i + 1 < iChainSize; ++i)
			Bones[Chain[i]]->Set_TransformationMatrix(XMLoadFloat4x4(&SavedLocals[i]));
		Context.pOwner->Update_ForwardKinematics(iRoot);

		if (fPen < 0.f && iDeep >= 0)
		{
			const _float fSkin = 0.02f;
			vSolveTarget = vPlanePoint + (-fPen + fSkin) * vPlaneNormal;
		}
	}

	Solve_FabrikPosition(Context, vSolveTarget, Target.Runtime.fCurWeight);
	Context.pOwner->Update_ForwardKinematics(iRoot);
	return tResult;
}

// Forward(End => Root) / Backward(Root => End) 왕복 반복
_uint CIKSolver_Fabrik::Iterate_Fabrik(const IK_SOLVE_CONTEXT& Context, _fvector vTarget, _fvector vRootOrigin, _fvector vPlaneNormal, _float& fErrOut)
{
	const _float fEPSILON = 1e-6f;
	const IK_TARGET& Target = *Context.pTarget;
	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;
	const IK_OPTION_FABRIK& OptFabrik = Target.OptFabrik;

	vector<_vector>& P = WS.Positions;
	const vector<_float>& L = WS.Lengths;
	const size_t iN = P.size();

	fErrOut = FLT_MAX;

	_bool bPlane = (XMVectorGetX(XMVector3LengthSq(vPlaneNormal)) > fEPSILON);

	_vector vLastDir = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	auto SafeDir = [&](_fvector vFrom, _fvector vTo) -> _vector {
		_vector vDiff = vTo - vFrom;
		if (bPlane)
			vDiff = vDiff - XMVector3Dot(vDiff, vPlaneNormal) * vPlaneNormal;
		if (XMVectorGetX(XMVector3LengthSq(vDiff)) < fEPSILON)
			return vLastDir;
		vLastDir = XMVector3Normalize(vDiff);
		return vLastDir;
	};

	_uint iIterDone = 0;
	for (_uint i = 0; i < OptFabrik.iMaxIterations; ++i)
	{
		// Forward : End => Root.
		P[iN - 1] = vTarget;
		for (size_t k = iN - 1; k-- > 0; )	// k = iN-2 .. 0 (size_t는 k >= 0이 항상 참이므로 이 관용구 사용)
			P[k] = P[k + 1] + SafeDir(P[k + 1], P[k]) * L[k];

		// Backward : Root => End.
		P[0] = vRootOrigin;
		for (size_t k = 1; k < iN; ++k)
			P[k] = P[k - 1] + SafeDir(P[k - 1], P[k]) * L[k - 1];

		++iIterDone;
		fErrOut = XMVectorGetX(XMVector3Length(P[iN - 1] - vTarget));
		if (fErrOut < OptFabrik.fTolerance)
			break;
	}

	return iIterDone;
}

void CIKSolver_Fabrik::Solve_FabrikPosition(const IK_SOLVE_CONTEXT& Context, _fvector vTargetPos, _float fWeight)
{
	const _float fEPSILON = 1e-6f;
	const IK_TARGET& Target = *Context.pTarget;
	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;

	// 1. 위치 지정(현재 위치)
	if (!Gather_Chain(Context))
		return;

	vector<_vector>& P = WS.Positions;
	const vector<_float>& L = WS.Lengths;
	const size_t iN = P.size();

	_vector vRootOrigin = P[0];
	_float d = XMVectorGetX(XMVector3Length(vTargetPos - vRootOrigin));

	_vector vAxis = vTargetPos - vRootOrigin;
	_vector vPlaneNormal = XMVectorZero();
	if (XMVectorGetX(XMVector3LengthSq(vAxis)) > fEPSILON && iN >= 3)
	{
		vAxis = XMVector3Normalize(vAxis);
		_vector vElbow = P[iN - 2];
		_vector vPoleDir = CIKSolver::TwoBoneMakePoleVector(vRootOrigin, vElbow, vTargetPos);

		IK_TARGET_RUNTIME& RT = const_cast<IK_TARGET_RUNTIME&>(Target.Runtime);
		if (RT.bHasPole)
		{
			if (XMVectorGetX(XMVector3Dot(vPoleDir, RT.vLastPoleDir)) < 0.f)
				vPoleDir = XMVectorNegate(vPoleDir);
			vPoleDir = XMVector3Normalize(XMVectorLerp(RT.vLastPoleDir, vPoleDir, 0.25f));
		}
		RT.vLastPoleDir = vPoleDir;
		RT.bHasPole = true;

		vPlaneNormal = XMVector3Normalize(XMVector3Cross(vAxis, vPoleDir));
	}

	if (d >= WS.fTotalLength - fEPSILON)
	{
		for (size_t i = 0; i + 1 < iN; ++i)
		{
			_float r = XMVectorGetX(XMVector3Length(vTargetPos - P[i]));
			if (r < fEPSILON)
			{
				P[i + 1] = vTargetPos;
				continue;
			}
			_float lambda = L[i] / r;
			P[i + 1] = XMVectorLerp(P[i], vTargetPos, lambda);
		}
	}
	else
	{
		_float fErr = 0.f;
		Iterate_Fabrik(Context, vTargetPos, vRootOrigin, vPlaneNormal, fErr);
	}

	// 위치해 → 본 회전 재구성 (FABRIK/CCD 공용 베이스 헬퍼)
	Apply_ChainPositions(Context, fWeight);
}

CIKSolver_Fabrik* CIKSolver_Fabrik::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSolver_Fabrik* pInstance = new CIKSolver_Fabrik(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_Fabrik");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSolver_Fabrik::Clone(void* pArg)
{
	CIKSolver_Fabrik* pClone = new CIKSolver_Fabrik(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_Fabrik (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSolver_Fabrik::Free()
{
	__super::Free();
}

#include "EnginePch.h"
#include "Engine_Profile.h"
#include "IKSolver_Fabrik.h"
#include "GameObject.h"
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
	PROFILE_ZONE(); // Release에서만 실행될 때 동작함.
	IK_RESULT tResult = Update_InverseKinematics(Context);

	

	return tResult;
}

IK_RESULT CIKSolver_Fabrik::Update_InverseKinematics(const IK_SOLVE_CONTEXT& Context)
{
	IK_RESULT tResult{};
	tResult.isSolved = true;

	// 1. 제약 조건
	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;

	_uint iChainSize = static_cast<_uint>(Chain.size());
	if (iChainSize < 3)
		return tResult;
	if (Target.Runtime.fCurWeight <= 0.f)
		return tResult;

	// 2. 확인.
	if (!Gather_Chain(Context))
	{
		tResult.isSolved = false;
		return tResult;
	}
		

	// 3. 타겟 위치 추출
	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;
	_vector vTargetPos = Target.Runtime.vCurTargetPos; // 모델 스페이스 공간으로 변환되어서 전달 받음.
	_vector vRootOriginPos = WS.Positions[0];

	// 4. 분기.


	
}

_bool CIKSolver_Fabrik::Gather_Chain(const IK_SOLVE_CONTEXT& Context)
{
	const _float fEPSILON = 1e-6f;
	const vector<CBone*>& Bones = *Context.pBones;
	const IK_TARGET& Target = *Context.pTarget;
	const vector<_uint>& Chain = Target.Chain.BoneChain;

	_uint iChainSize = static_cast<_uint>(Chain.size());

	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;
	// 워크스페이스는 타겟들이 돌려쓰므로 총길이까지 매번 초기화해야 합니다.
	WS.Positions.clear();
	WS.Lengths.clear();
	WS.fTotalLength = 0.f;

	// 2. Bone Chaing 으로 부터 정보를 추출합니다.
	for (_uint i = 0; i < iChainSize; i++)
	{
		// 위치 저장.
		_matrix matComb = XMLoadFloat4x4(Bones[Chain[i]]->Get_CombinedTransformationMatrix());
		WS.Positions.push_back(matComb.r[3]);
	}

	for (_uint i = 0; i < iChainSize - 1; i++)
	{
		_float fLength = XMVectorGetX(XMVector3Length(WS.Positions[i] - WS.Positions[i + 1]));
		if (fLength < fEPSILON)
			return false;
		WS.Lengths.push_back(fLength);
		WS.fTotalLength += fLength;
	}

	return true;
}

// Forward(End→Root) / Backward(Root→End) 왕복 반복. 수행한 반복 횟수를 반환합니다.
_uint CIKSolver_Fabrik::Iterate_Fabrik(const IK_SOLVE_CONTEXT& Context, _fvector vTarget, _fvector vRootOrigin, _float& fErrOut)
{
	const _float fEPSILON = 1e-6f;
	const IK_TARGET& Target = *Context.pTarget;
	IK_SOLVE_WORKSPACE& WS = *Context.pWorkspace;
	const IK_OPTION_FABRIK& OptFabrik = Target.OptFabrik;

	vector<_vector>& P = WS.Positions;
	const vector<_float>& L = WS.Lengths;
	const size_t iN = P.size();

	fErrOut = FLT_MAX;

	// 관절이 겹쳐 방향을 잃으면 직전 유효 방향을 재사용합니다. (Normalize의 NaN 방어)
	_vector vLastDir = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	auto SafeDir = [&](_fvector vFrom, _fvector vTo) -> _vector {
		_vector vDiff = vTo - vFrom;
		if (XMVectorGetX(XMVector3LengthSq(vDiff)) < fEPSILON)
			return vLastDir;
		vLastDir = XMVector3Normalize(vDiff);
		return vLastDir;
	};

	_uint iIterDone = 0;
	for (_uint i = 0; i < OptFabrik.iMaxIterations; ++i)
	{
		// Forward : End => Root. 엔드를 타겟에 붙이고 길이를 보존하며 거슬러 올라갑니다.
		P[iN - 1] = vTarget;
		for (size_t k = iN - 1; k-- > 0; )	// k = iN-2 .. 0 (size_t는 k >= 0이 항상 참이므로 이 관용구 사용)
			P[k] = P[k + 1] + SafeDir(P[k + 1], P[k]) * L[k];

		// Backward : Root => End. 루트를 제자리로 되돌리고 다시 내려옵니다.
		P[0] = vRootOrigin;
		for (size_t k = 1; k < iN; ++k)
			P[k] = P[k - 1] + SafeDir(P[k - 1], P[k]) * L[k - 1];

		// 오차는 Backward 이후에 측정해야 의미가 있습니다.
		// (Forward 직후엔 P[iN-1]이 타겟 그 자체라 항상 0)
		++iIterDone;
		fErrOut = XMVectorGetX(XMVector3Length(P[iN - 1] - vTarget));
		if (fErrOut < OptFabrik.fTolerance)
			break;
	}

	return iIterDone;
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

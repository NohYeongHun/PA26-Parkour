#include "EnginePch.h"
#include "IKSolver.h"
#include "GameInstance.h"
#include "Bone.h"

CIKSolver::CIKSolver(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice {pDevice}, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

CIKSolver::CIKSolver(const CIKSolver& Prototype)
	: m_pDevice{ Prototype.m_pDevice }, m_pContext{ Prototype.m_pContext },
	m_pGameInstance{ CGameInstance::GetInstance() },
	m_isClone { true }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CIKSolver::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSolver::Initialize_Clone(void* pArg)
{
	return S_OK;
}

HRESULT CIKSolver::Render()
{
	return S_OK;
}
_vector CIKSolver::TwoBoneMakePoleVector(_fvector vRootPos, _fvector vMidPos, _vector vEndPos, _float Distance)
{
	const _float fEPSILON = 1e-6f;

	// 1. Root -> End 축
	_vector vAxis = XMVector3Normalize(vEndPos - vRootPos);

	// 2. 관절이 축에서 벗어난 방향
	_vector vMidPoint = (vRootPos + vEndPos) * 0.5f;
	_vector vBendDir = vMidPos - vMidPoint;

	// 3. 축 성분을 제거하여 축에 순수 직교하는 성분만 남깁니다.
	_float vDot = XMVectorGetX(XMVector3Dot(vBendDir, vAxis));
	vBendDir = vBendDir - vDot * vAxis; // 이건 vRoot와 vEnd의 중점에서 굽어있는 수직 벡터를 구하기 위함.

	// 4. 관절이 거의 직선 상에 있어 방향이 불안정한 경우
	_vector vLenSq = XMVector3LengthSq(vBendDir);
	if (XMVectorGetX(vLenSq) < fEPSILON)
	{
		// 축에 직교하는 안정적인 방향 하나를 선택.
		
		_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		vBendDir = XMVector3Cross(vAxis, vUp);

		// 월드 Up 과 축의 외적을 시도하고, 평행하면 Right 로 대체.
		if (XMVectorGetX(XMVector3LengthSq(vBendDir)) < fEPSILON)
		{
			_vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
			vBendDir = XMVector3Cross(vAxis, vRight);
		}
	}

	vBendDir = XMVector3Normalize(vBendDir);

	return vBendDir;
}


_float CIKSolver::Measure_DeepestPenetration(const vector<CBone*>& Bones, const vector<_uint>& EndSubtree, _fvector vPlanePoint, _fvector vPlaneNormal, _int& iDeepestOut)
{
	iDeepestOut = -1;
	_float fMin = FLT_MAX;

	// 자손 판정은 Register_Target에서 캐시된 목록 사용 (매 프레임 조상 탐색 제거)
	for (_uint i : EndSubtree)
	{
		_vector vPos = XMLoadFloat4x4(Bones[i]->Get_CombinedTransformationMatrix()).r[3];
		_float fPen = XMVectorGetX(XMVector3Dot(vPos - vPlanePoint, vPlaneNormal));
		if (fPen < fMin)
		{
			fMin = fPen;
			iDeepestOut = static_cast<_int>(i);
		}
	}

	if (iDeepestOut < 0)
		return 0.f;

	return fMin;
}

_bool CIKSolver::Is_Descendant(const vector<CBone*>& Bones, _uint iBone, _uint iAncestor)
{
	_int iParent = Bones[iBone]->Get_ParentIndex();
	while (iParent >= 0)
	{
		if (static_cast<_uint>(iParent) == iAncestor)
			return true;
		iParent = Bones[static_cast<_uint>(iParent)]->Get_ParentIndex();
	}

	return false;
}

void CIKSolver::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

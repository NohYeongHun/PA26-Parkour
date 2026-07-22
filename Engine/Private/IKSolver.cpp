#include "EnginePch.h"
#include "IKSolver.h"
#include "GameInstance.h"

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


void CIKSolver::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

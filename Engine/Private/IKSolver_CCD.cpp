#include "EnginePch.h"
#include "IKSolver_CCD.h"
#include "GameObject.h"

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
	return IK_RESULT();
}

const _char* CIKSolver_CCD::Get_Name() const
{
	return nullptr;
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

#include "EnginePch.h"
#include "IKSolver_Fabrik.h"
#include "GameObject.h"

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
	return IK_RESULT();
}

const _char* CIKSolver_Fabrik::Get_Name() const
{
	return nullptr;
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

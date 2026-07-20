#include "EnginePch.h"
#include "IKSovler_Fabrik.h"
#include "GameObject.h"

CIKSovler_Fabrik::CIKSovler_Fabrik(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSovler_Fabrik::CIKSovler_Fabrik(const CIKSovler_Fabrik& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSovler_Fabrik::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSovler_Fabrik::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSovler_Fabrik::Render()
{
	return S_OK;
}

IK_RESULT CIKSovler_Fabrik::Solve(_float fTimeDelta)
{
}

const _char* CIKSovler_Fabrik::Get_Name() const
{
	return nullptr;
}

CIKSovler_Fabrik* CIKSovler_Fabrik::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSovler_Fabrik* pInstance = new CIKSovler_Fabrik(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_Fabrik");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSovler_Fabrik::Clone(void* pArg)
{
	CIKSovler_Fabrik* pClone = new CIKSovler_Fabrik(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_Fabrik (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSovler_Fabrik::Free()
{
	__super::Free();
}

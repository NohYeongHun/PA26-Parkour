#include "EnginePch.h"
#include "IKSovler_CCD.h"
#include "GameObject.h"

CIKSovler_CCD::CIKSovler_CCD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSovler_CCD::CIKSovler_CCD(const CIKSovler_CCD& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSovler_CCD::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSovler_CCD::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSovler_CCD::Render()
{
	return S_OK;
}

IK_RESULT CIKSovler_CCD::Solve(_float fTimeDelta)
{
	return IK_RESULT{};
}

const _char* CIKSovler_CCD::Get_Name() const
{
	return nullptr;
}

CIKSovler_CCD* CIKSovler_CCD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSovler_CCD* pInstance = new CIKSovler_CCD(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_CCD");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSovler_CCD::Clone(void* pArg)
{
	CIKSovler_CCD* pClone = new CIKSovler_CCD(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_CCD (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSovler_CCD::Free()
{
	__super::Free();
}

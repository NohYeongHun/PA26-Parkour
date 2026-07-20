#include "EnginePch.h"
#include "IKSovler_TwoBone.h"
#include "GameObject.h"

CIKSovler_TwoBone::CIKSovler_TwoBone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CIKSolver { pDevice, pContext }
{
}

CIKSovler_TwoBone::CIKSovler_TwoBone(const CIKSovler_TwoBone& Prototype)
	: CIKSolver(Prototype)
{
}

HRESULT CIKSovler_TwoBone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKSovler_TwoBone::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIKSovler_TwoBone::Render()
{
	return S_OK;
}

IK_RESULT CIKSovler_TwoBone::Solve(_float fTimeDelta)
{
}

const _char* CIKSovler_TwoBone::Get_Name() const
{
	return nullptr;
}

CIKSovler_TwoBone* CIKSovler_TwoBone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKSovler_TwoBone* pInstance = new CIKSovler_TwoBone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : IKSovler_TwoBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CIKSolver* CIKSovler_TwoBone::Clone(void* pArg)
{
	CIKSovler_TwoBone* pClone = new CIKSovler_TwoBone(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : IKSovler_TwoBone (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CIKSovler_TwoBone::Free()
{
	__super::Free();
}

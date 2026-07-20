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


void CIKSolver::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

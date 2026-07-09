#include "ClientPch.h"
#include "ClimbState.h"

HRESULT CClimbState::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CClimbState::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
}

void CClimbState::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);
}

void CClimbState::OnExit()
{
	__super::OnExit();
}


void CClimbState::Free()
{
	__super::Free();
}

#include "ClientPch.h"
#include "AirState.h"

HRESULT CAirState::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CAirState::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
}

void CAirState::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);
}

void CAirState::OnExit()
{
	__super::OnExit();
}


void CAirState::Free()
{
	__super::Free();
}

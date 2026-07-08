#include "ClientPch.h"
#include "GroundState.h"

HRESULT CGroundState::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CGroundState::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
}

void CGroundState::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);
}

void CGroundState::OnExit()
{
	__super::OnExit();
}


void CGroundState::Free()
{
	__super::Free();
}

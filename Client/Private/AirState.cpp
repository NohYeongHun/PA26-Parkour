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

_bool CAirState::Play_Animation(_float fTimeDelta)
{
	const auto& iter = m_Animations.find(m_iCurrentAnimIdx);
	if (iter == m_Animations.end())
		return false;

	m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(
		iter->second.AnimPlayDesc, iter->second.RootMotionDesc, fTimeDelta);

	// 공중 상태: 루트 모션 시각 동기화만 하고 물리 위치는 건드리지 않음
	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);

	return true;
}

void CAirState::OnExit()
{
	__super::OnExit();
}


void CAirState::Free()
{
	__super::Free();
}

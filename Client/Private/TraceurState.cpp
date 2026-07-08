#include "ClientPch.h"
#include "TraceurState.h"
#include "Traceur.h"
#include "Model.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"


HRESULT CTraceurState::Initialize(CTraceur* pOwner)
{
	m_pOwner = pOwner;
	/* Component 캐싱 */
	if (nullptr == pOwner)
		return E_FAIL;

	m_pModelCom = dynamic_cast<CModel*>(pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	m_pColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pColliderCom)
		return E_FAIL;

	m_pStateMachinCom = dynamic_cast<CStateMachine*>(m_pOwner->Get_Component(TEXT("Com_StateMachine")));
	if (nullptr == m_pStateMachinCom)
		return E_FAIL;

	m_pInputControllerCom = dynamic_cast<CInputController*>(m_pOwner->Get_Component(TEXT("Com_InputController")));
	if (nullptr == m_pInputControllerCom)
		return E_FAIL;

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	if (nullptr == m_pEnvQueryCom)
		return E_FAIL;

	m_pMoveCom = dynamic_cast<CMovementComponent*>(m_pOwner->Get_Component(TEXT("Com_Move")));
	if (nullptr == m_pMoveCom)
		return E_FAIL;

	// 자주 사용하는 이동 키는 저장.
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);

	return S_OK;
}

void CTraceurState::OnEnter(void* pArg)
{
	CState::OnEnter(pArg);
}

void CTraceurState::OnUpdate(_float fTimeDelta)
{
	CState::OnUpdate(fTimeDelta);
}

void CTraceurState::OnExit()
{
	CState::OnExit();
}

_bool CTraceurState::Play_Animation(_float fTimeDelta)
{
	const auto& iter = m_Animations.find(m_iCurrentAnimIdx);
	if (iter == m_Animations.end())
		return false;

	if (iter->second.eType == EAnimSlotType::BLENDSPACE_1D)
	{
		m_pModelCom->Play_BlendSpace_CPU(iter->second.BlendSpaceDesc, iter->second.RootMotionDesc, fTimeDelta);
		m_IsAnimationEnd = false;
	}
	else
	{
		m_IsAnimationEnd = m_pModelCom->Play_Animation_CPU(iter->second.AnimPlayDesc, iter->second.RootMotionDesc, fTimeDelta);
	}

	m_pModelCom->Sync_RootNode(m_pTransformCom, fTimeDelta);
	return true;
}


void CTraceurState::Free()
{
	__super::Free();
}

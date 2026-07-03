#include "ClientPch.h"
#include "Client_CharacterEnum.h"
#include "MovementComponent.h"
#include "InputController.h"
#include "SpringCamera.h"


CMovementComponent::CMovementComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CMovementComponent::CMovementComponent(const CMovementComponent& Prototype)
	: CComponent (Prototype )
{
}

HRESULT CMovementComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMovementComponent::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	ASSERT_CRASH(m_pOwner);
	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	ASSERT_CRASH(m_pTransformCom);

	return S_OK;
}



ACTORDIR CMovementComponent::Calculate_Direction(const CInputController* pInputController) const
{
	if (nullptr == pInputController)
		return ACTORDIR::END;

	_bool bW = pInputController->Check_AnyInput(ENUM_CLASS(KEYINPUT::W));
	_bool bS = pInputController->Check_AnyInput(ENUM_CLASS(KEYINPUT::S));
	_bool bA = pInputController->Check_AnyInput(ENUM_CLASS(KEYINPUT::A));
	_bool bD = pInputController->Check_AnyInput(ENUM_CLASS(KEYINPUT::D));

	if (bW && bA)      return ACTORDIR::LU;
	else if (bW && bD) return ACTORDIR::RU;
	else if (bS && bA) return ACTORDIR::LD;
	else if (bS && bD) return ACTORDIR::RD;
	else if (bW)       return ACTORDIR::U;
	else if (bS)       return ACTORDIR::D;
	else if (bA)       return ACTORDIR::L;
	else if (bD)       return ACTORDIR::R;

	return ACTORDIR::END;
}

_vector CMovementComponent::Calculate_Move_Direction(const CSpringCamera* pSpringCamera, ACTORDIR eDir) const
{
	if (nullptr == pSpringCamera)
		return XMVectorZero();

	_vector vLook = pSpringCamera->Get_LookVector_NoPitch();
	_vector vRight = pSpringCamera->Get_RightVector_NoPitch();

	switch (eDir)
	{
		case ACTORDIR::U:   return vLook;
		case ACTORDIR::D:   return -vLook;
		case ACTORDIR::L:   return -vRight;
		case ACTORDIR::R:   return vRight;
		case ACTORDIR::LU:  return XMVector3Normalize(vLook - vRight);
		case ACTORDIR::LD:  return XMVector3Normalize(-vLook - vRight);
		case ACTORDIR::RU:  return XMVector3Normalize(vLook + vRight);
		case ACTORDIR::RD:  return XMVector3Normalize(-vLook + vRight);
		default: return XMVectorZero();
	}

	return XMVectorZero();
}

void CMovementComponent::Move_Direction(const CInputController* pInputController, const CSpringCamera* pSpringCamera,  _float fTimeDelta, _float fSpeed) const
{
	if (nullptr == pInputController || 
		nullptr == pSpringCamera || 
		nullptr == m_pTransformCom)
		return;

	ACTORDIR eDir = Calculate_Direction(pInputController);
	_vector vMoveDir = Calculate_Move_Direction(pSpringCamera, eDir);
	m_pTransformCom->LookLerp(vMoveDir, fTimeDelta, 10.f);
	m_pTransformCom->Go_Dir(vMoveDir * fSpeed, fTimeDelta);
}

void CMovementComponent::Move_Direction(_fvector vDir, _float fTimeDelta, _float fSpeed)
{
	if (nullptr == m_pTransformCom)
		return;

	m_pTransformCom->Go_Dir(vDir * fSpeed, fTimeDelta);
}


CMovementComponent* CMovementComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMovementComponent* pInstance = new CMovementComponent(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MovementComponent");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMovementComponent::Clone(void* pArg)
{
	CMovementComponent* pClone = new CMovementComponent(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MovementComponent (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMovementComponent::Free()
{
	__super::Free();
	m_pTransformCom = nullptr;
}

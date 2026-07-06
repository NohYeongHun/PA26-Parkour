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

ACTORDIR CMovementComponent::Calculate_Direction(const CInputController* pInputController)
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

_vector CMovementComponent::Calc_WorldDir(ACTORDIR eDir, _fvector vCamForward, _fvector vCamRight)
{

	switch (eDir)
	{
		case ACTORDIR::U:   return vCamForward;
		case ACTORDIR::D:   return -vCamForward;
		case ACTORDIR::L:   return -vCamRight;
		case ACTORDIR::R:   return vCamRight;
		case ACTORDIR::LU:  return XMVector3Normalize(vCamForward - vCamRight);
		case ACTORDIR::LD:  return XMVector3Normalize(-vCamForward - vCamRight);
		case ACTORDIR::RU:  return XMVector3Normalize(vCamForward + vCamRight);
		case ACTORDIR::RD:  return XMVector3Normalize(-vCamForward + vCamRight);
		default: return XMVectorZero();
	}

	return XMVectorZero();
}

void CMovementComponent::Move(_fvector vWorldDir, _float fTimeDelta, _float fSpeed)
{
	if (XMVector3Equal(vWorldDir, XMVectorZero()))
		return;

	m_pTransformCom->LookLerp(vWorldDir, fTimeDelta, 10.f);
	m_pTransformCom->Go_Dir(vWorldDir * fSpeed, fTimeDelta);
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

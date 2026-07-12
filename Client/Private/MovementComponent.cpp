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

	m_eMoventType = MOVEMENT_TYPE::GROUND;

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

_vector CMovementComponent::Calc_GroundDir(ACTORDIR eDir, _fvector vCamForward, _fvector vCamRight)
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

_vector CMovementComponent::Calc_ClimbDir(ACTORDIR eDir, _fvector vClimbNormal, _fvector vWorldUp)
{
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, -vClimbNormal));
	//_vector vRight = XMVector3Normalize(XMVector3Cross(vClimbNormal, vWorldUp));
	_vector vUp = -XMVector3Normalize(XMVector3Cross(vClimbNormal, vRight));


	switch (eDir)
	{
		case ACTORDIR::U:   return vUp;
		case ACTORDIR::D:   return -vUp;
		case ACTORDIR::L:   return -vRight;
		case ACTORDIR::R:   return vRight;
		case ACTORDIR::LU:  return XMVector3Normalize(vUp - vRight);
		case ACTORDIR::LD:  return XMVector3Normalize(-vUp - vRight);
		case ACTORDIR::RU:  return XMVector3Normalize(vUp + vRight);
		case ACTORDIR::RD:  return XMVector3Normalize(-vUp + vRight);
		default: return XMVectorZero();
	}

	return XMVectorZero();
}

void CMovementComponent::Move(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight)
{
	switch (m_eMoventType)
	{
	case MOVEMENT_TYPE::GROUND:
		GroundMove(vWorldDir, fTimeDelta, fTargetWeight);
		break;
	case MOVEMENT_TYPE::CLIMB:
		ClimbMove(vWorldDir, fTimeDelta, fTargetWeight);
		break;
	}
	
}


void CMovementComponent::GroundMove(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight)
{
	_bool bHasInput = !XMVector3Equal(vWorldDir, XMVectorZero());
	_float fTarget = bHasInput ? fTargetWeight : 0.f;
	_float fSmoothTime = (fTarget > m_fLocomotionWeight) ? m_fAccelTime : m_fDecelTime;
	_float fRate = (fSmoothTime > 0.f) ? min(fTimeDelta / fSmoothTime, 1.f) : 1.f;
	m_fLocomotionWeight += (fTarget - m_fLocomotionWeight) * fRate;

	if (m_fLocomotionWeight < 0.01f)
	{
		m_fLocomotionWeight = 0.f;
		return;
	}

	if (bHasInput)
	{
		XMStoreFloat3(&m_vLastMoveDir, vWorldDir);
		m_pTransformCom->LookLerp(vWorldDir, fTimeDelta, 3.f);
	}

	_fvector vMoveDir = bHasInput ? vWorldDir : XMLoadFloat3(&m_vLastMoveDir);
	m_pTransformCom->Go_Dir(vMoveDir * (m_fLocomotionWeight * m_fMaxMoveSpeed), fTimeDelta);
}

void CMovementComponent::ClimbMove(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight)
{
	_bool bHasInput = !XMVector3Equal(vWorldDir, XMVectorZero());
	
	_float fTarget = bHasInput ? fTargetWeight : 0.f;
	_float fSmoothTime = (fTarget > m_fLocomotionWeight) ? m_fAccelTime : m_fDecelTime;
	_float fRate = (fSmoothTime > 0.f) ? min(fTimeDelta / fSmoothTime, 1.f) : 1.f;
	m_fLocomotionWeight += (fTarget - m_fLocomotionWeight) * fRate;

	// 입력이 있을 때만 실제로 이동. (LookLerp는 벽 수직 방향에서 축이 깨지므로 사용하지 않음)
	if (bHasInput)
	{
		XMStoreFloat3(&m_vLastClimbDir, vWorldDir);
		m_pTransformCom->Go_Dir(vWorldDir * m_fMaxClimbSpeed, fTimeDelta);
	}
}

void CMovementComponent::Update_ClimbBlendWeight(ACTORDIR eDir, _float fTimeDelta)
{
	_float2 vTarget = {};
	switch (eDir)
	{
		case ACTORDIR::U:  vTarget = { 0.f,  1.f }; break;
		case ACTORDIR::D:  vTarget = { 0.f, -1.f }; break;
		case ACTORDIR::L:  vTarget = { -1.f, 0.f }; break;
		case ACTORDIR::R:  vTarget = { 1.f,  0.f }; break;
		case ACTORDIR::LU: vTarget = { -1.f, 1.f }; break;
		case ACTORDIR::LD: vTarget = { -1.f,-1.f }; break;
		case ACTORDIR::RU: vTarget = { 1.f,  1.f }; break;
		case ACTORDIR::RD: vTarget = { 1.f, -1.f }; break;
		default:           vTarget = { 0.f,  0.f }; break; // END: 입력 없음 -> Idle
	}

	_bool bHasInput = (eDir != ACTORDIR::END);
	_float fSmoothTime = bHasInput ? m_fAccelTime : m_fDecelTime;
	_float fRate = (fSmoothTime > 0.f) ? min(fTimeDelta / fSmoothTime, 1.f) : 1.f;

	m_fLocomotionWeight2D.x += (vTarget.x - m_fLocomotionWeight2D.x) * fRate;
	m_fLocomotionWeight2D.y += (vTarget.y - m_fLocomotionWeight2D.y) * fRate;
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

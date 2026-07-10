#include "ClientPch.h"
#include "Character.h"
#include "MovementComponent.h"
#include "InputController.h"

CCharacter::CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
	: CGameObject (Prototype)
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	if (FAILED(CGameObject::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCharacter::Initialize_Clone(void* pArg)
{
	CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);
	
	m_eCurLevel = pDesc->eCurLevel;

	if (FAILED(CGameObject::Initialize_Clone(pDesc)))
		return E_FAIL;


	return S_OK;
}

void CCharacter::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CCharacter::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

void CCharacter::Render()
{
}

void CCharacter::Move(ACTORDIR eDir, const _fvector& vCamForward, const _fvector& vCamRight, _float fTimeDelta, _float fSpeed)
{
	_vector vMoveDir = CMovementComponent::Calc_GroundDir(eDir, vCamForward, vCamRight);
	m_pMoveCom->Move(vMoveDir, fTimeDelta, fSpeed);
}

HRESULT CCharacter::Ready_Components(const CHARACTER_DESC* pDesc)
{
	
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
		, pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
		, pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");


	if (FAILED(Ready_MovementComponents(pDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CCharacter::Ready_MovementComponents(const CHARACTER_DESC* pDesc)
{
	/* MoveComp */
	CMovementComponent::COMPONENT_DESC MoveCompDesc{};
	MoveCompDesc.pOwner = this;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Movement"),
		TEXT("Com_Move"), reinterpret_cast<CComponent**>(&m_pMoveCom), &MoveCompDesc)))
		return E_FAIL;

	return S_OK;
}



void CCharacter::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pMoveCom);
}

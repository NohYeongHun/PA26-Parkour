#include "ClientPch.h"
#include "Character.h"
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

HRESULT CCharacter::Ready_Components(const CHARACTER_DESC* pDesc)
{
	
	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
		, pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("Model");

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
		, pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("Shader");


	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->inputControllerData.first)
		, pDesc->inputControllerData.second, TEXT("Com_InputController"), reinterpret_cast<CComponent**>(&m_pInputControllerCom), nullptr)))
		CRASH("Input Controller");

	


	return S_OK;
}



void CCharacter::Free()
{
	__super::Free();
	
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pInputControllerCom);
}

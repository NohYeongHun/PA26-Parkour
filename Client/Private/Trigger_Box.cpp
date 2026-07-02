#include "ClientPch.h"
#include "Trigger_Box.h"
#include "GameSystem.h"
#include "Event_Level.h"

CTrigger_Box::CTrigger_Box(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

CTrigger_Box::CTrigger_Box(const CTrigger_Box& Prototype)
	:CGameObject(Prototype), m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CTrigger_Box::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger_Box::Initialize_Clone(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	m_iTriggerIndex = pDesc->iTriggerIndex;

	Ready_Components(pArg);
	Register_Trigger();
	
	return S_OK;
}

void CTrigger_Box::Priority_Update(_float fTimeDelta)
{
	
}

void CTrigger_Box::Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CTrigger_Box::Late_Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Render();
}

void CTrigger_Box::Ready_Components(void* pArg)
{
	TRIGGER* pDesc = static_cast<TRIGGER*>(pArg);

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Kinematic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	RigidbodyDesc.vExtent = pDesc->vExtends;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
	
}

void CTrigger_Box::Collision_Enter()
{
	if (m_IsTriggered)
		return;

	
}

void CTrigger_Box::Collision_During()
{
	
}

void CTrigger_Box::Collision_End()
{
}

void CTrigger_Box::Register_Trigger()
{
	
}

void CTrigger_Box::UI_Set(_bool B)
{
}


CTrigger_Box* CTrigger_Box::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrigger_Box* pInstance = new CTrigger_Box(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTrigger_Box::Clone(void* pArg)
{
	CTrigger_Box* pInstance = new CTrigger_Box(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_TriggerBox (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrigger_Box::Free()
{
	__super::Free();
	m_pTempPtr = nullptr;
	m_pSecondTempPtr= nullptr;
	Safe_Delete(m_CamMatrix);
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pRigidbodyCom);
}
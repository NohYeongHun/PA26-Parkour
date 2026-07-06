#include"ClientPch.h"
#include "Slide_Navigation.h"

CSlide_Navigation::CSlide_Navigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CSlide_Navigation::CSlide_Navigation(const CSlide_Navigation& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CSlide_Navigation::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSlide_Navigation::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);
    return S_OK;
}

void CSlide_Navigation::Priority_Update(_float fTimeDelta)
{
}

void CSlide_Navigation::Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CSlide_Navigation::Late_Update(_float fTimeDelta)
{
}

void CSlide_Navigation::Render()
{
}

void CSlide_Navigation::Ready_Component(void* pArg)
{
	//SLIDE_NAVIGATION_DESC* pDesc = static_cast<SLIDE_NAVIGATION_DESC*>(pArg);
	//CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eBodyType = CRigidbody::BODY;
	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.eType = EMotionType::Kinematic;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::SLIDE);
	//RigidbodyDesc.vExtent = pDesc->vExtends; // 탐지 범위는 적절하게. 설정해주기.
	//m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMat));
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));

	//if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc)))
	//	CRASH("Rigidbody");

	//m_CallBack.IsStart = pDesc->IsStart;
	//if (m_CallBack.IsStart)
	//{
	//	vector<_float3> WayPoint;
	//	for (_uint i = 0; i < pDesc->iPathSize; ++i)
	//	{
	//		_float3 CurrentPath = _float3(pDesc->pPath[i].x, pDesc->pPath[i].y, pDesc->pPath[i].z);
	//		WayPoint.push_back(CurrentPath);
	//	}
	//	m_CallBack.eSlideData.WayPoints = move(WayPoint);
	//}
	//m_CallBack.pTransform = m_pTransformCom;

	//m_pRigidbodyCom->Set_Desc(&m_CallBack);
}

CSlide_Navigation* CSlide_Navigation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlide_Navigation* pInstance = new CSlide_Navigation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Model");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSlide_Navigation::Clone(void* pArg)
{
	CSlide_Navigation* pClone = new CSlide_Navigation(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Model (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CSlide_Navigation::Free()
{
	__super::Free();
	Safe_Release(m_pRigidbodyCom);
}

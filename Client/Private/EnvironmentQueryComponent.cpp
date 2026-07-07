#include "ClientPch.h"
#include "EnvironmentQueryComponent.h"
#include "Traceur.h"


CEnvironmentQueryComponent::CEnvironmentQueryComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CEnvironmentQueryComponent::CEnvironmentQueryComponent(const CEnvironmentQueryComponent& Prototype)
	: CComponent ( Prototype )
{
}

HRESULT CEnvironmentQueryComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEnvironmentQueryComponent::Initialize_Clone(void* pArg)
{

	const ENV_QUERY_DESC* pDesc = static_cast<ENV_QUERY_DESC*>(pArg);
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (nullptr == m_pOwner)
		return E_FAIL;

	m_pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pOwnerTransformCom)
		return E_FAIL;

	m_pOwnerColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pOwnerColliderCom)
		return E_FAIL;
	
	m_fShapeTraceDistance = pDesc->fShapeTraceDistance;
	m_fLineTraceDistance = pDesc->fLineTraceDistance;
	m_eTargetLayer = pDesc->eTargetLayer;

	return S_OK;
}

void CEnvironmentQueryComponent::Execute(_float fTimeDelta)
{
	if (Detect_Obstacle(fTimeDelta))
	{
		Detect_ObstacleShape(fTimeDelta);
	}
	

}

// 1. 1차적으로 Owner가 가지고 있는 Shape를 통한 충돌 가능성을 파악합니다.
_bool CEnvironmentQueryComponent::Detect_Obstacle(_float fTimeDelta)
{
	m_OutHit = {};
	_bool isHit = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), m_pOwnerTransformCom->Get_Quaternion(),
			m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset(),
			m_pOwnerTransformCom->Get_State(STATE::LOOK), m_fShapeTraceDistance, ENUM_CLASS(m_eTargetLayer), m_OutHit);

	if (isHit)
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(m_OutHit.pDesc);
		m_eParkourFlag = pDesc->eParkourFlag; // 디자이너가 설정한 파쿠르 플래그를 받습니다. 기본(ALL)
	}

	return isHit;
}


// 2. Ray Cast 를 통한 지형 지물의 정보를 저장합니다.
void CEnvironmentQueryComponent::Detect_ObstacleShape(_float fTimeDelta)
{
	_vector vLook = XMVector3Normalize(m_pOwnerTransformCom->Get_State(STATE::LOOK));
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_float fTotalHeight = m_pOwnerColliderCom->Get_Height() + 2.f * m_pOwnerColliderCom->Get_Radius();
	
	_vector vBottom = vCenter - XMVectorSet(0.f, fTotalHeight * 0.5f, 0.f, 0.f);
	_vector vChestStart = vBottom + (vLook * m_pOwnerColliderCom->Get_Radius()) + XMVectorSet(0.f, fTotalHeight * FCHEST_RATIO, 0.f, 0.f);
	_vector vHeadStart = vBottom + XMVectorSet(0.f, fTotalHeight * FHEAD_RATIO, 0.f, 0.f);

	// 1. Bottom
	m_BottomHit = Ray_Cast(vBottom, vBottom + (vLook * m_fLineTraceDistance));
	// 2. Chest
	m_ChestHit = Ray_Cast(vChestStart, vChestStart + vLook * (m_fLineTraceDistance - m_pOwnerColliderCom->Get_Radius()));
	// 3. Head
	m_HeadHit = Ray_Cast(vHeadStart, vHeadStart + (vLook * m_fLineTraceDistance));

#ifdef _DEBUG
	if (m_BottomHit.isHit)
		cout << "Bottom Hit" << endl;
	if (m_ChestHit.isHit)
		cout << "Chest Hit" << endl;
	if (m_HeadHit.isHit)
		cout << "Head Hit" << endl;
	cout << endl;

	
#endif // _DEBUG

}



CEnvironmentQueryComponent::LINE_TRACE_HIT CEnvironmentQueryComponent::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos)
{
	LINE_TRACE_HIT lineTrace;
	lineTrace.isHit = m_pGameInstance->Ray_Cast(vStartPos, vEndPos, ENUM_CLASS(m_eTargetLayer), &lineTrace.vHitPoint);
	return lineTrace;
}

CEnvironmentQueryComponent* CEnvironmentQueryComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEnvironmentQueryComponent* pInstance = new CEnvironmentQueryComponent(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CEnvironmentQueryComponent");
		Safe_Release(pInstance);
	}

	return pInstance;
}

Engine::CComponent* CEnvironmentQueryComponent::Clone(void* pArg)
{
	CEnvironmentQueryComponent* pClone = new CEnvironmentQueryComponent(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : EnvironmentQueryComponent (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEnvironmentQueryComponent::Free()
{
	__super::Free();
	m_pOwnerTransformCom = nullptr;
	m_pOwnerColliderCom = nullptr;
}

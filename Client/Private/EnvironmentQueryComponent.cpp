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
		Collect_RayInfo(fTimeDelta);
		Classify_HitFlag();
		Judge_Condition();
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
		m_eObjectParkourFlag = pDesc->eObjectParkourFlag; // 디자이너가 설정한 파쿠르 플래그를 받습니다. 기본(ALL)
	}

	return isHit;
}


// 2. Ray Cast 를 통한 지형 지물의 정보를 저장합니다.
void CEnvironmentQueryComponent::Collect_RayInfo(_float fTimeDelta)
{
	_vector vLook = XMVector3Normalize(m_pOwnerTransformCom->Get_State(STATE::LOOK));
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_float fTotalHeight = m_pOwnerColliderCom->Get_Height() + 2.f * m_pOwnerColliderCom->Get_Radius();
	
	_vector vBottom = vCenter - XMVectorSet(0.f, fTotalHeight * 0.5f, 0.f, 0.f);
	_vector vKneeStart       = vBottom + XMVectorSet(0.f, fTotalHeight * FKNEE_RATIO, 0.f, 0.f);
	_vector vChestStart = vBottom + XMVectorSet(0.f, fTotalHeight * FCHEST_RATIO, 0.f, 0.f);
	_vector vHeadStart  = vBottom + XMVectorSet(0.f, fTotalHeight * FHEAD_RATIO, 0.f, 0.f);

	// 1. Bottom
	m_KneeHit = Ray_Cast(vKneeStart, vKneeStart + (vLook * m_fLineTraceDistance));
	// 2. Chest
	m_ChestHit = Ray_Cast(vChestStart, vChestStart + vLook * (m_fLineTraceDistance));
	// 3. Head
	m_HeadHit = Ray_Cast(vHeadStart, vHeadStart + (vLook * m_fLineTraceDistance));


	// 4. Top RayCast
	const LINE_TRACE_HIT& TopHit = m_HeadHit.isHit ? m_HeadHit
		: m_ChestHit.isHit ? m_ChestHit : m_KneeHit;
	_vector vHitPos = XMLoadFloat3(&TopHit.vHitPosition);
	_float fInset = m_pOwnerColliderCom->Get_Radius() * 0.5f; // 밀어 넣을 깊이.
	_vector vStartXZ = vHitPos + XMVector3Normalize(vLook) * fInset;

	_float fStartY = XMVectorGetY(vBottom) + fTotalHeight * FMAX_REACH_RATIO;
	_vector vDownStart = XMVectorSetY(vStartXZ, fStartY);
	_vector vDownEnd = XMVectorSetY(vStartXZ, XMVectorGetY(vBottom));

	RAY_CAST_HIT TopDownRay = m_pGameInstance->Ray_Cast(vDownStart, vDownEnd, ENUM_CLASS(m_eTargetLayer));


#ifdef _DEBUG
	Print_Debug();
#endif // _DEBUG

	
}

#ifdef _DEBUG
void CEnvironmentQueryComponent::Print_Debug()
{
	if (m_KneeHit.isHit)
		cout << "Knee Hit" << endl;
	if (m_ChestHit.isHit)
		cout << "Chest Hit" << endl;
	if (m_HeadHit.isHit)
		cout << "Head Hit" << endl;
	cout << endl;
}
#endif // _DEBUG





CEnvironmentQueryComponent::LINE_TRACE_HIT CEnvironmentQueryComponent::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos)
{
	LINE_TRACE_HIT lineTrace;
	//lineTrace.isHit = m_pGameInstance->Ray_Cast(vStartPos, vEndPos, ENUM_CLASS(m_eTargetLayer), &lineTrace.vHitPoint);
	RAY_CAST_HIT RayCastHit = m_pGameInstance->Ray_Cast(vStartPos, vEndPos, ENUM_CLASS(m_eTargetLayer));
	if (RayCastHit.isHit)
	{
		lineTrace.isHit = true;
		lineTrace.vHitPosition = RayCastHit.vHitPosition;
		lineTrace.vHitNormal = RayCastHit.vHitNormal;
		lineTrace.fCenterDistance = RayCastHit.fDistance; // 센터 기준 Distance
	}

	return lineTrace;
}



void CEnvironmentQueryComponent::Classify_HitFlag()
{
	m_iHeightFlag = 0;
	if (m_KneeHit.isHit)  m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE);
	if (m_ChestHit.isHit) m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST);
	if (m_HeadHit.isHit)  m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD);
}

void CEnvironmentQueryComponent::Start_DownRayCast()
{
	


}



void CEnvironmentQueryComponent::Judge_Condition()
{
	_uint iObjectParkourTag = ENUM_CLASS(m_eObjectParkourFlag);
	// 1. ParkourTag는 현재 어떤 작업을 선택 가능한지 여부를 가지고 있음.

	// 2. HeightFlag와 조합하여 후호분을 조합한다.

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

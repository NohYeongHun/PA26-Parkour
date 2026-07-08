#include "ClientPch.h"
#include "EnvironmentQueryComponent.h"


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
	if (nullptr == pArg)
		return E_FAIL;

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

	const ENV_QUERY_DESC* pDesc = static_cast<ENV_QUERY_DESC*>(pArg);
	m_fShapeTraceDistance = pDesc->fShapeTraceDistance;
	m_fLineTraceDistance = pDesc->fLineTraceDistance;
	m_eTargetLayer = pDesc->eTargetLayer;

	return S_OK;
}

void CEnvironmentQueryComponent::Execute()
{
	m_EnvQueryResult = {};

	if (Detect_Obstacle())
	{
		Collect_RayInfo();
		Classify_HitFlag();
		Extract_Geometry();
		Judge_Condition();

//#ifdef _DEBUG
//		Print_Debug();
//#endif
	}
}

// 1차적으로 Owner가 가지고 있는 Shape를 통한 충돌 가능성을 파악합니다.
_bool CEnvironmentQueryComponent::Detect_Obstacle()
{
	SHAPE_CAST_HIT OutHit{};
	_bool isHit = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), m_pOwnerTransformCom->Get_Quaternion(),
			m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset(),
			m_pOwnerTransformCom->Get_State(STATE::LOOK), m_fShapeTraceDistance, ENUM_CLASS(m_eTargetLayer), OutHit);

	if (isHit)
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(OutHit.pDesc);
		m_eObjectParkourFlag = pDesc->eObjectParkourFlag; // 디자이너가 설정한 파쿠르 플래그를 받습니다.
	}

	return isHit;
}

// Ray Cast 를 통한 지형 지물의 정보를 저장합니다.
void CEnvironmentQueryComponent::Collect_RayInfo()
{
	_vector vLook = XMVector3Normalize(m_pOwnerTransformCom->Get_State(STATE::LOOK));
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_float fTotalHeight = m_pOwnerColliderCom->Get_Height() + 2.f * m_pOwnerColliderCom->Get_Radius();

	_vector vBottom     = vCenter - XMVectorSet(0.f, fTotalHeight * 0.5f, 0.f, 0.f);
	_vector vKneeStart  = vBottom + XMVectorSet(0.f, fTotalHeight * FKNEE_RATIO,  0.f, 0.f);
	_vector vChestStart = vBottom + XMVectorSet(0.f, fTotalHeight * FCHEST_RATIO, 0.f, 0.f);
	_vector vHeadStart  = vBottom + XMVectorSet(0.f, fTotalHeight * FHEAD_RATIO,  0.f, 0.f);

	m_KneeHit  = Ray_Cast(vKneeStart,  vKneeStart  + vLook * m_fLineTraceDistance);
	m_ChestHit = Ray_Cast(vChestStart, vChestStart + vLook * m_fLineTraceDistance);
	m_HeadHit  = Ray_Cast(vHeadStart,  vHeadStart  + vLook * m_fLineTraceDistance);
}

CEnvironmentQueryComponent::LINE_TRACE_HIT CEnvironmentQueryComponent::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos)
{
	LINE_TRACE_HIT lineTrace;
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

// 수평 레이의 Hit 패턴을 비트마스크로 분류합니다.
void CEnvironmentQueryComponent::Classify_HitFlag()
{
	m_iHeightFlag = 0;
	if (m_KneeHit.isHit)  m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE);
	if (m_ChestHit.isHit) m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST);
	if (m_HeadHit.isHit)  m_iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD);
}

// 수직 레이로 장애물의 상단면·두께·착지 공간을 추출합니다.
void CEnvironmentQueryComponent::Extract_Geometry()
{
	if (m_iHeightFlag == 0) return;

	_vector vLook = XMVector3Normalize(m_pOwnerTransformCom->Get_State(STATE::LOOK));
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_float fRadius = m_pOwnerColliderCom->Get_Radius();
	_float fTotalHeight = m_pOwnerColliderCom->Get_Height() + 2.f * fRadius;
	_vector vBottom = vCenter - XMVectorSet(0.f, fTotalHeight * 0.5f, 0.f, 0.f);

	// 가장 높이 히트한 수평 레이의 지점에서 안쪽으로 밀어 Down Ray 시작점 결정
	const LINE_TRACE_HIT& TopHit = m_HeadHit.isHit ? m_HeadHit
		: m_ChestHit.isHit ? m_ChestHit : m_KneeHit;

	_float fInset = fRadius * 0.5f;
	_vector vStartXZ = XMLoadFloat3(&TopHit.vHitPosition) + vLook * fInset;

	_float fStartY = XMVectorGetY(vBottom) + fTotalHeight * FMAX_REACH_RATIO;
	_vector vDownStart = XMVectorSetY(vStartXZ, fStartY);
	_vector vDownEnd   = XMVectorSetY(vStartXZ, XMVectorGetY(vBottom));

	// 상단면 Down Ray의 위치, 노멀, 높이 값 저장
	RAY_CAST_HIT TopDownRay = m_pGameInstance->Ray_Cast(vDownStart, vDownEnd, ENUM_CLASS(m_eTargetLayer));
	OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;

	// 전면 히트: 가장 낮은 수평 레이 (VAULT 판정용 거리 계산에 사용)
	const LINE_TRACE_HIT& FrontHit = m_KneeHit.isHit ? m_KneeHit
		: m_ChestHit.isHit ? m_ChestHit : m_HeadHit;
	if (FrontHit.isHit)
	{
		Geo.vFrontHitPos = FrontHit.vHitPosition;
		Geo.vFrontNormal = FrontHit.vHitNormal;
	}

	if (!TopDownRay.isHit)
	{
		// 상단이 최대 도달 높이보다 위 — 모서리 정보 없이 벽타기 시작 판정용 기하만 기록
		Geo.isTopReachable  = false;
		Geo.fObstacleHeight = fTotalHeight * FMAX_REACH_RATIO;
		return;
	}

	Geo.isTopReachable  = true;
	Geo.vTopEdgePos     = TopDownRay.vHitPosition;
	Geo.vTopNormal      = TopDownRay.vHitNormal;
	Geo.fObstacleHeight = TopDownRay.vHitPosition.y - XMVectorGetY(vBottom);

	// 두께 탐지 — 상단면 위에서 전방으로 Down RayCast를 발사.
	_float fTopSurfaceY = TopDownRay.vHitPosition.y;
	_float fSampleStep = fRadius;
	Geo.fDepth = fInset; // TopDownRay 지점의 깊이 파악
	for (_uint i = 1; i <= FDEPTH_SAMPLE_COUNT; ++i)
	{
		_vector vSample = vStartXZ + vLook * (fSampleStep * static_cast<_float>(i));
		_vector vSStart = XMVectorSetY(vSample, fStartY);
		_vector vSEnd   = XMVectorSetY(vSample, fTopSurfaceY);
		if (!m_pGameInstance->Ray_Cast(vSStart, vSEnd, ENUM_CLASS(m_eTargetLayer)).isHit)
			break;
		Geo.fDepth += fSampleStep;
	}

	// 착지점 탐지 — 뒷모서리 너머 아래로 긴 레이 1개

	_vector vLandXZ    = vStartXZ + vLook * (Geo.fDepth + fRadius);
	_vector vLandStart = XMVectorSetY(vLandXZ, fTopSurfaceY + 0.05f);
	_vector vLandEnd   = XMVectorSetY(vLandXZ, XMVectorGetY(vBottom) - fTotalHeight);

	// 바닥은 MAP 레이어
	RAY_CAST_HIT LandRayHit = m_pGameInstance->Ray_Cast(vLandStart, vLandEnd, ENUM_CLASS(m_eTargetLayer));
	if (false == LandRayHit.isHit)
		LandRayHit = m_pGameInstance->Ray_Cast(vLandStart, vLandEnd, ENUM_CLASS(COLLISIONLAYER::MAP));

	Geo.hasLandingSpace = LandRayHit.isHit;
	if (LandRayHit.isHit)
		Geo.vLandingPos = LandRayHit.vHitPosition;
}

// LineTrace의 Hit 패턴, 정해진 파쿠르 태그, 기하 정보로 최종 액션 판정.
void CEnvironmentQueryComponent::Judge_Condition()
{
	// 기하 정보가 없으면 판정 불가
	if (m_EnvQueryResult.Geometry.fObstacleHeight <= 0.f) return;

	// 높이 패턴 1차 후보
	const bool bKnee  = (m_iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE))  != 0;
	const bool bChest = (m_iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST)) != 0;
	const bool bHead  = (m_iHeightFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD))  != 0;

	_uint iCandidateFlag = 0;
	if (bKnee && !bHead)
		iCandidateFlag = ENUM_CLASS(PARKOUR_FLAG::VAULTABLE) | ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE);
	else if (bKnee && bChest && bHead)
		iCandidateFlag = ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE);

	// 디자이너 PARKOUR_FLAG 적용 — END는 ALL로 처리
	_uint iParkourMask = ENUM_CLASS(m_eObjectParkourFlag);
	if (iParkourMask >= ENUM_CLASS(PARKOUR_FLAG::END))
		iParkourMask = ENUM_CLASS(PARKOUR_FLAG::ALL);
	iCandidateFlag &= iParkourMask;

	// 기하 정보로 세부 필터
	_float fRadius = m_pOwnerColliderCom->Get_Radius();
	_float fTotalHeight = m_pOwnerColliderCom->Get_Height() + 2.f * fRadius;
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;

	// 상단 미도달 벽은 모서리 기반 동작(VAULT/MANTLE) 불가 — 벽타기 CLIMB만 남김
	if (!Geo.isTopReachable)
		iCandidateFlag &= ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE);

	if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::VAULTABLE))
		if (Geo.fDepth > fRadius * FVAULT_MAX_DEPTH_MULT || !Geo.hasLandingSpace)
			iCandidateFlag &= ~ENUM_CLASS(PARKOUR_FLAG::VAULTABLE);

	if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE))
		if (Geo.fDepth < fRadius * FMANTLE_MIN_DEPTH_MULT)
			iCandidateFlag &= ~ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE);

	// 높이 제한은 모서리를 잡고 오르는 경우에만 적용 — 상단 미도달 벽은 벽타기로 무제한 허용
	if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE))
		if (Geo.isTopReachable && Geo.fObstacleHeight > fTotalHeight * FMAX_CLIMBABLE_HEIGHT_RATIO)
			iCandidateFlag &= ~ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE);

	// 우선순위 결정 (VAULT > MANTLE > CLIMB)
	m_EnvQueryResult.iCandidateFlag = iCandidateFlag;
	m_EnvQueryResult.isValid        = false;
	m_EnvQueryResult.eBestAction    = PARKOUR_ACTION::NONE;

	if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::VAULTABLE))
	{
		// 가슴 레이까지 히트한 높은 장애물은 HIGH_VAULT로 구분
		m_EnvQueryResult.eBestAction = bChest ? PARKOUR_ACTION::HIGH_VAULT : PARKOUR_ACTION::VAULT;
		m_EnvQueryResult.isValid     = true;
	}
	else if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::MANTLEABLE))
	{
		m_EnvQueryResult.eBestAction = PARKOUR_ACTION::MANTLE;
		m_EnvQueryResult.isValid     = true;
	}
	else if (iCandidateFlag & ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE))
	{
		m_EnvQueryResult.eBestAction = PARKOUR_ACTION::CLIMB;
		m_EnvQueryResult.isValid     = true;
	}
}

#ifdef _DEBUG
void CEnvironmentQueryComponent::Print_Debug()
{
	if (m_KneeHit.isHit)  cout << "Knee Hit"  << endl;
	if (m_ChestHit.isHit) cout << "Chest Hit" << endl;
	if (m_HeadHit.isHit)  cout << "Head Hit"  << endl;

	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	if (Geo.fObstacleHeight > 0.f)
	{
		cout << "[Geometry] H=" << Geo.fObstacleHeight
		     << " D=" << Geo.fDepth
		     << " Landing=" << Geo.hasLandingSpace
		     << " TopReach=" << Geo.isTopReachable << endl;
	}

	if (m_EnvQueryResult.isValid)
	{
		static const char* s_ActionNames[] = { "NONE", "VAULT", "HIGH_VAULT", "MANTLE", "CLIMB", "HANG" };
		cout << "[Action] " << s_ActionNames[ENUM_CLASS(m_EnvQueryResult.eBestAction)] << endl;
	}

	cout << endl;
}
#endif // _DEBUG

_bool CEnvironmentQueryComponent::Find_Ground(const _fvector& vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos)
{
	_vector vStart = vProbePos + XMVectorSet(0.f, fUpOffset, 0.f, 0.f);
	_vector vEnd   = vProbePos - XMVectorSet(0.f, fMaxDrop,  0.f, 0.f);

	RAY_CAST_HIT hit = m_pGameInstance->Ray_Cast(vStart, vEnd, ENUM_CLASS(m_eTargetLayer));
	if (!hit.isHit)
		hit = m_pGameInstance->Ray_Cast(vStart, vEnd, ENUM_CLASS(COLLISIONLAYER::MAP));

	if (hit.isHit)
	{
		vOutGroundPos = hit.vHitPosition;
		return true;
	}
	return false;
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

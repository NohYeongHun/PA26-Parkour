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

	if (nullptr == pDesc->pBodyProfile)
		return E_FAIL;
	m_pBodyProfile = pDesc->pBodyProfile;

	return S_OK;
}

void CEnvironmentQueryComponent::Set_ScanDirOverride(_fvector vDir)
{
	_vector vXZ = XMVectorSetY(vDir, 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vXZ)) < 1e-6f)
		return; // 무의미한 방향 — 기존 오버라이드/LOOK 유지
	XMStoreFloat3(&m_vScanDirOverride, XMVector3Normalize(vXZ));
	m_hasScanDirOverride = true;
}

_vector CEnvironmentQueryComponent::Get_ScanDir() const
{
	if (m_hasScanDirOverride)
		return XMLoadFloat3(&m_vScanDirOverride);
	return XMVector3Normalize(m_pOwnerTransformCom->Get_State(STATE::LOOK));
}

void CEnvironmentQueryComponent::Execute()
{
	m_Perception = {};

	if (!Detect_Obstacle())
		return;

	Scan_Obstacle();
	Measure_Geometry();

#ifdef _DEBUG
	Draw_DebugMarkers();
#endif // _DEBUG
}

// [1단계] Owner의 Shape로 전방 충돌 가능성과 디자이너 태그를 파악합니다.
_bool CEnvironmentQueryComponent::Detect_Obstacle()
{
	SHAPE_CAST_HIT ShapeHit{};
	const _vector vCastQuat = m_hasScanDirOverride ? XMQuaternionIdentity() : m_pOwnerTransformCom->Get_Quaternion();
	_bool isHit = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), vCastQuat,
			m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset(),
			Get_ScanDir(), m_fShapeTraceDistance, ENUM_CLASS(m_eTargetLayer), ShapeHit);

	m_Perception.Scan.isObstacleDetected = isHit;
	if (isHit && (nullptr != ShapeHit.pDesc))
	{
		CALLBACK_CLIENT* pDesc = static_cast<CALLBACK_CLIENT*>(ShapeHit.pDesc);
		m_Perception.Scan.eObjectFlag = pDesc->eObjectParkourFlag;
	}

	return isHit;
}

// 모든 레이 캐스트의 단일 창구 — 디버그 빌드에서 자동 시각화되므로 레이를 빠뜨리고 그릴 수 없다.
LINE_TRACE_HIT CEnvironmentQueryComponent::Cast_Ray(const _fvector& vStart, const _fvector& vEnd, _uint iLayer, RAY_KIND eKind)
{
	LINE_TRACE_HIT lineTrace;
	RAY_CAST_HIT RayCastHit = m_pGameInstance->Ray_Cast(vStart, vEnd, iLayer);
	if (RayCastHit.isHit)
	{
		lineTrace.isHit = true;
		lineTrace.vHitPosition = RayCastHit.vHitPosition;
		lineTrace.vHitNormal = RayCastHit.vHitNormal;
		lineTrace.fCenterDistance = RayCastHit.fDistance; // 센터 기준 Distance
	}

#ifdef _DEBUG
	if (m_pGameInstance->IsParkourDebug())
	{
		JPH::Color color = JPH::Color(128.f, 128.f, 128.f, 1.f); // 미스 = 회색
		if (lineTrace.isHit)
		{
			switch (eKind)
			{
			case RAY_KIND::SCAN:    color = JPH::Color(0.f, 255.f, 0.f, 1.f);   break;
			case RAY_KIND::MEASURE: color = JPH::Color(0.f, 0.f, 255.f, 1.f);   break;
			case RAY_KIND::REFINE:  color = JPH::Color(0.f, 255.f, 255.f, 1.f); break;
			}
		}
		m_pGameInstance->Add_DebugLine(vStart, lineTrace.isHit ? XMLoadFloat3(&lineTrace.vHitPosition) : vEnd, color);
	}
#endif // _DEBUG

	return lineTrace;
}

// [2단계] 수평 레이 5개의 히트 정보와 높이 패턴 비트마스크를 기록합니다.
void CEnvironmentQueryComponent::Scan_Obstacle()
{
	OBSTACLE_SCAN& Scan = m_Perception.Scan;

	_vector vLook = Get_ScanDir();
	XMStoreFloat3(&Scan.vScanDir, vLook);
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));

	_vector vBottom     = vCenter - XMVectorSet(0.f, m_pBodyProfile->fHeight * 0.5f, 0.f, 0.f);
	_vector vKneeStart  = vBottom + XMVectorSet(0.f, m_pBodyProfile->fKneeHeight,  0.f, 0.f);
	_vector vChestStart = vBottom + XMVectorSet(0.f, m_pBodyProfile->fChestHeight, 0.f, 0.f);
	_vector vHeadStart  = vBottom + XMVectorSet(0.f, m_pBodyProfile->fHeadHeight,  0.f, 0.f);
	_vector vLeftChestStart  = vChestStart - (vRight * m_pBodyProfile->fRadius);
	_vector vRightChestStart = vChestStart + (vRight * m_pBodyProfile->fRadius);

	Scan.KneeHit  = Cast_Ray(vKneeStart,  vKneeStart  + vLook * m_fLineTraceDistance, ENUM_CLASS(m_eTargetLayer), RAY_KIND::SCAN);
	Scan.ChestHit = Cast_Ray(vChestStart, vChestStart + vLook * m_fLineTraceDistance, ENUM_CLASS(m_eTargetLayer), RAY_KIND::SCAN);
	Scan.HeadHit  = Cast_Ray(vHeadStart,  vHeadStart  + vLook * m_fLineTraceDistance, ENUM_CLASS(m_eTargetLayer), RAY_KIND::SCAN);
	Scan.LeftChestHit  = Cast_Ray(vLeftChestStart,  vLeftChestStart  + (vLook * m_fLineTraceDistance), ENUM_CLASS(m_eTargetLayer), RAY_KIND::SCAN);
	Scan.RightChestHit = Cast_Ray(vRightChestStart, vRightChestStart + (vLook * m_fLineTraceDistance), ENUM_CLASS(m_eTargetLayer), RAY_KIND::SCAN);

	Scan.iHeightFlag = 0;
	if (Scan.KneeHit.isHit)  Scan.iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE);
	if (Scan.ChestHit.isHit) Scan.iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::CHEST);
	if (Scan.HeadHit.isHit)  Scan.iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD);
}

// 수직 레이로 장애물의 상단면·두께·착지 공간을 추출합니다.
void CEnvironmentQueryComponent::Measure_Geometry()
{
	const OBSTACLE_SCAN& Scan = m_Perception.Scan;
	if (Scan.iHeightFlag == 0) return;

	_vector vLook = Get_ScanDir();
	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_float fRadius = m_pBodyProfile->fRadius;
	_float fTotalHeight = m_pBodyProfile->fHeight;
	_vector vBottom = vCenter - XMVectorSet(0.f, fTotalHeight * 0.5f, 0.f, 0.f);

	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	// 가장 높이 히트한 수평 레이의 지점에서 안쪽으로 밀어 Down Ray 시작점 결정
	const LINE_TRACE_HIT& TopHit = Scan.HeadHit.isHit ? Scan.HeadHit
		: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.KneeHit;

	// 전면 히트: 가장 낮은 수평 레이
	const LINE_TRACE_HIT& FrontHit = Scan.KneeHit.isHit ? Scan.KneeHit
		: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.HeadHit;

	_vector vTraversal = vLook;
	if (FrontHit.isHit)
	{
		Geo.hasFront = true;
		Geo.vFrontHitPos = FrontHit.vHitPosition;
		Geo.vFrontNormal = FrontHit.vHitNormal;

		_vector vNormalXZ = XMVectorSetY(XMLoadFloat3(&Geo.vFrontNormal), 0.f);
		if (XMVectorGetX(XMVector3LengthSq(vNormalXZ)) > 1e-4f) 
			vTraversal = -XMVector3Normalize(vNormalXZ);

		Geo.fFrontDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&Geo.vFrontHitPos) - m_pOwnerTransformCom->Get_State(STATE::POSITION)));
	}
	XMStoreFloat3(&Geo.vTraversalDir, vTraversal);

	_float fInset = fRadius * 0.5f;
	_vector vStartXZ = XMLoadFloat3(&TopHit.vHitPosition) + vTraversal * fInset;

	_float fStartY = XMVectorGetY(vBottom) + m_pBodyProfile->fMaxReach;
	_vector vDownStart = XMVectorSetY(vStartXZ, fStartY);
	_vector vDownEnd = XMVectorSetY(vStartXZ, XMVectorGetY(vBottom));

	LINE_TRACE_HIT TopDownRay = Cast_Ray(vDownStart, vDownEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);

	if (!TopDownRay.isHit)
	{
		Geo.isTopReachable  = false;
		Geo.fObstacleHeight = m_pBodyProfile->fMaxReach;
		return;
	}

	if (TopDownRay.vHitPosition.y <= TopHit.vHitPosition.y)
	{
		Geo.isTopReachable  = false;
		Geo.fObstacleHeight = m_pBodyProfile->fMaxReach;
		return;
	}

	Geo.isTopReachable  = true;
	Geo.vTopNormal      = TopDownRay.vHitNormal;
	Geo.fObstacleHeight = TopDownRay.vHitPosition.y - XMVectorGetY(vBottom);

	if (Geo.hasFront)
	{
		_vector vFrontXZ = XMLoadFloat3(&Geo.vFrontHitPos);
		Geo.vTopEdgePos = _float3(XMVectorGetX(vFrontXZ), TopDownRay.vHitPosition.y, XMVectorGetZ(vFrontXZ));
	}
	else
		Geo.vTopEdgePos = TopDownRay.vHitPosition; // 전면 정보가 없으면 종전 방식 폴백

	_vector vSideAxis = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vTraversal));
	Geo.fTopWidth = 0.f;
	{
		const _float SideOffsets[2] = { -fRadius, fRadius };
		for (_uint i = 0; i < 2; ++i)
		{
			_vector vWSample = vStartXZ + vSideAxis * SideOffsets[i];
			_vector vWStart  = XMVectorSetY(vWSample, fStartY);
			_vector vWEnd    = XMVectorSetY(vWSample, TopDownRay.vHitPosition.y);
			if (Cast_Ray(vWStart, vWEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE).isHit)
				Geo.fTopWidth += fRadius;
		}
	}

	_float fTopSurfaceY = TopDownRay.vHitPosition.y;
	_float fSampleStep  = fRadius;

	_float fLastHit   = 0.f;  // vStartXZ 기준, 마지막으로 상단면이 확인된 전방 거리
	_float fFirstMiss = -1.f; // 첫 미스 거리 (-1 = 탐지 범위 내 미스 없음)
	for (_uint i = 1; i <= FDEPTH_SAMPLE_COUNT; ++i)
	{
		_float  fDist   = fSampleStep * static_cast<_float>(i);
		_vector vSample = vStartXZ + vTraversal * fDist;
		_vector vSStart = XMVectorSetY(vSample, fStartY);
		_vector vSEnd   = XMVectorSetY(vSample, fTopSurfaceY);
		if (Cast_Ray(vSStart, vSEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE).isHit)
			fLastHit = fDist;
		else
		{
			fFirstMiss = fDist;
			break;
		}
	}

	if (fFirstMiss < 0.f)
	{
		// 탐지 범위 끝까지 상단면이 이어짐 — 두께는 하한값만 보장 (기존과 동일한 값)
		Geo.fDepth   = fInset + fSampleStep * static_cast<_float>(FDEPTH_SAMPLE_COUNT);
		Geo.hasDepth = true;
	}
	else
	{
		_float fLo = fLastHit;
		_float fHi = fFirstMiss;
		for (_uint i = 0; i < FEDGE_REFINE_ITERATIONS; ++i)
		{
			_float  fMid    = (fLo + fHi) * 0.5f;
			_vector vSample = vStartXZ + vTraversal * fMid;
			_vector vSStart = XMVectorSetY(vSample, fStartY);
			_vector vSEnd   = XMVectorSetY(vSample, fTopSurfaceY);
			if (Cast_Ray(vSStart, vSEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::REFINE).isHit)
				fLo = fMid;
			else
				fHi = fMid;
		}
		Geo.fDepth   = fInset + (fLo + fHi) * 0.5f;
		Geo.hasDepth = true;
	}

	if (Geo.isTopReachable && Geo.hasDepth)
	{
		const _float fStandMargin = 0.15f;
		_float fStandInset = (Geo.fDepth >= 2.f * fRadius)
			? std::clamp(fRadius + fStandMargin, fRadius, Geo.fDepth - fRadius)
			: Geo.fDepth * 0.5f;

		_vector vStandXZ = XMLoadFloat3(&Geo.vTopEdgePos) + vTraversal * fStandInset;

		_vector vStandStart = XMVectorSetY(vStandXZ, fStartY);
		_vector vStandEnd = XMVectorSetY(vStandXZ, XMVectorGetY(vBottom));
		LINE_TRACE_HIT StandHit = Cast_Ray(vStandStart, vStandEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);

		if (StandHit.isHit)
			Geo.vTopStandPos = StandHit.vHitPosition;
		else
			Geo.vTopStandPos = _float3(XMVectorGetX(vStandXZ), fTopSurfaceY, XMVectorGetZ(vStandXZ));
	}


	// 착지점 탐지 — 뒷모서리 너머 아래로 긴 레이 1개 + 평탄성 검증 레이 2개
	_vector vLandXZ    = vStartXZ + vTraversal * (Geo.fDepth + fRadius);
	_vector vLandStart = XMVectorSetY(vLandXZ, fTopSurfaceY + 0.05f);
	_vector vLandEnd   = XMVectorSetY(vLandXZ, XMVectorGetY(vBottom) - fTotalHeight);

	// 바닥은 MAP 레이어 폴백
	LINE_TRACE_HIT LandRayHit = Cast_Ray(vLandStart, vLandEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);
	if (false == LandRayHit.isHit)
		LandRayHit = Cast_Ray(vLandStart, vLandEnd, ENUM_CLASS(COLLISIONLAYER::MAP), RAY_KIND::MEASURE);

	Geo.hasLandingSpace = LandRayHit.isHit;
	if (LandRayHit.isHit)
	{
		Geo.vLandingPos = LandRayHit.vHitPosition;

		// 착지점 전후가 낭떠러지·급경사면 착지 공간으로 보지 않는다
		const _float ProbeOffsets[2] = { -fRadius * 0.75f, fRadius * 0.75f };
		for (_uint i = 0; i < 2; ++i)
		{
			_vector vProbeXZ = vLandXZ + vTraversal * ProbeOffsets[i];
			_vector vPStart  = XMVectorSetY(vProbeXZ, fTopSurfaceY + 0.05f);
			_vector vPEnd    = XMVectorSetY(vProbeXZ, XMVectorGetY(vBottom) - fTotalHeight);

			LINE_TRACE_HIT ProbeHit = Cast_Ray(vPStart, vPEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);
			if (false == ProbeHit.isHit)
				ProbeHit = Cast_Ray(vPStart, vPEnd, ENUM_CLASS(COLLISIONLAYER::MAP), RAY_KIND::MEASURE);

			if (!ProbeHit.isHit || fabsf(ProbeHit.vHitPosition.y - Geo.vLandingPos.y) > FLANDING_MAX_HEIGHT_DIFF)
			{
				Geo.hasLandingSpace = false;
				break;
			}
		}
	}
}


#ifdef _DEBUG
void CEnvironmentQueryComponent::Draw_DebugMarkers()
{
	if (!m_pGameInstance->IsParkourDebug())
		return;

	const OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	if (Geo.isTopReachable)
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&Geo.vTopEdgePos), 0.07f, JPH::Color(255.f, 0.f, 0.f, 1.f));
	if (Geo.hasLandingSpace)
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&Geo.vLandingPos), 0.07f, JPH::Color(255.f, 255.f, 0.f, 1.f));
}
#endif // _DEBUG

#ifdef _DEBUG
void CEnvironmentQueryComponent::Print_Debug()
{
	const OBSTACLE_SCAN&     Scan = m_Perception.Scan;
	const OBSTACLE_GEOMETRY& Geo  = m_Perception.Geometry;

	cout << "[Scan]     Knee " << (Scan.KneeHit.isHit ? "O" : "X")
	     << "  Chest " << (Scan.ChestHit.isHit ? "O" : "X")
	     << "  Head " << (Scan.HeadHit.isHit ? "O" : "X")
	     << "  L " << (Scan.LeftChestHit.isHit ? "O" : "X")
	     << "  R " << (Scan.RightChestHit.isHit ? "O" : "X")
	     << "  ObjectFlag=" << ENUM_CLASS(Scan.eObjectFlag) << endl;

	cout << "[Geometry] H=" << Geo.fObstacleHeight
	     << " D=" << Geo.fDepth
	     << " W=" << Geo.fTopWidth
	     << " Front=" << Geo.fFrontDistance
	     << " Top=" << (Geo.isTopReachable ? "O" : "X")
	     << " Landing=" << (Geo.hasLandingSpace ? "O" : "X") << endl;
}
#endif // _DEBUG

_bool CEnvironmentQueryComponent::Find_Ground(const _fvector& vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos)
{
	_vector vStart = vProbePos + XMVectorSet(0.f, fUpOffset, 0.f, 0.f);
	_vector vEnd   = vProbePos - XMVectorSet(0.f, fMaxDrop,  0.f, 0.f);

	// 1차로 파쿠르 지형 RayCast
	RAY_CAST_HIT hit = m_pGameInstance->Ray_Cast(vStart, vEnd, ENUM_CLASS(m_eTargetLayer));

	// 2차로 Map 지형 RayCast
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

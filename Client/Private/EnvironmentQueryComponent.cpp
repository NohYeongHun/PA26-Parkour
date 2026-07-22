#include "ClientPch.h"
#include "EnvironmentQueryComponent.h"
#include "Engine_Profile.h"


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

_bool CEnvironmentQueryComponent::Resolve_Anchor(const _string& token, _vector& vOutPos, _vector& vOutNormal)
{
	const auto& Geo = m_Perception.Geometry;

	_vector vEdge = XMVectorSetW(XMLoadFloat3(&Geo.Top.vEdgePos), 1.f);
	_vector vTrav = XMLoadFloat3(&Geo.vTraversalDir);
	_vector vLat = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), vTrav)); // 엣지 좌우측
	const _float fSpacing = 0.35f;

	if (token == "TOP_LEFT_EDGE")
	{
		vOutPos = vEdge - vLat * fSpacing + XMVector3Normalize(vTrav) * 0.2f + XMVector3Normalize(XMLoadFloat3(&Geo.Top.vNormal)) * 0.19f;
		vOutNormal = XMLoadFloat3(&Geo.Top.vNormal);
		return Geo.Top.isReachable;
	}
	if (token == "TOP_RIGHT_EDGE")
	{
		vOutPos = vEdge + vLat * fSpacing + XMVector3Normalize(vTrav) * 0.2f + XMVector3Normalize(XMLoadFloat3(&Geo.Top.vNormal)) * 0.19f;
		vOutNormal = XMLoadFloat3(&Geo.Top.vNormal);
		return Geo.Top.isReachable;
	}

	if (token == "FORWARD_WALL")
	{
		vOutPos = XMLoadFloat3(&Geo.Front.vHitPos);
		vOutNormal = XMLoadFloat3(&Geo.Front.vNormal);
		return Geo.Front.hasHit;
	}

	return false;
}

void CEnvironmentQueryComponent::Set_ScanDirOverride(_fvector vDir)
{
	_vector vXZ = XMVectorSetY(vDir, 0.f);
	if (XMVectorGetX(XMVector3LengthSq(vXZ)) < 1e-6f)
		return;
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
	PROFILE_ZONE();
	m_Perception = {};

	Scan_Reach();

	if (!Detect_Obstacle())
		return;

	Scan_Obstacle();
	Measure_Geometry();
#ifdef _DEBUG
	if (m_pGameInstance->IsParkourDebug() && m_pGameInstance->IsDebugSphere())
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&m_Perception.Geometry.Top.vEdgePos), 0.1f, JPH::Color(0.f, 255.f, 0.f, 1.f));
#endif
#ifdef _DEBUG
	Draw_DebugMarkers();
#endif // _DEBUG
}

_bool CEnvironmentQueryComponent::Detect_Obstacle()
{
	SHAPE_CAST_HIT ShapeHit{};

	_vector vScanDir{};
	if (m_hasScanDirOverride)
		vScanDir = XMLoadFloat3(&m_vScanDirOverride);
	else
		vScanDir = m_pOwnerTransformCom->Get_State(STATE::LOOK);

	const _vector vCastQuat = m_hasScanDirOverride ? XMQuaternionIdentity() : m_pOwnerTransformCom->Get_Quaternion();
	const _vector vCastPos = m_pOwnerTransformCom->Get_State(STATE::POSITION)
		+ m_pOwnerColliderCom->Get_Offset();
	_bool isHit = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), vCastQuat,
			vCastPos, vScanDir, m_fShapeTraceDistance, ENUM_CLASS(m_eTargetLayer), ShapeHit);

	m_Perception.Scan.isObstacleDetected = isHit;
	if (isHit && (nullptr != ShapeHit.pDesc))
	{
		m_Perception.Scan.HitBodyID = ShapeHit.HitBodyID;
		m_Perception.Scan.eObjectFlag = static_cast<CALLBACK_CLIENT*>(ShapeHit.pDesc)->eObjectParkourFlag;

#ifdef _DEBUG
		Log_ShapeHit(ShapeHit);
#endif
	}

	return isHit;
}

LINE_TRACE_HIT CEnvironmentQueryComponent::Cast_Ray(_fvector vStart, _fvector vEnd, _uint iLayer, RAY_KIND eKind)
{
	LINE_TRACE_HIT lineTrace;
	RAY_CAST_HIT RayCastHit = m_pGameInstance->Ray_Cast(vStart, vEnd, iLayer);
	if (RayCastHit.isHit)
	{
		lineTrace.isHit = true;
		lineTrace.vHitPosition = RayCastHit.vHitPosition;
		lineTrace.vHitNormal = RayCastHit.vHitNormal;
		lineTrace.fCenterDistance = RayCastHit.fDistance;
	}

#ifdef _DEBUG
	if (m_pGameInstance->IsParkourDebug() && m_pGameInstance->IsDebugRay())
	{
		JPH::Color color = JPH::Color(128.f, 128.f, 128.f, 1.f);
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

LINE_TRACE_HIT CEnvironmentQueryComponent::Cast_Ray_WithMapFallback(_fvector vStart, _fvector vEnd, RAY_KIND eKind)
{
	// Land 구간을 찾으므로, Parkour Layer, Map Layer를 둘다 찾습니다.
	LINE_TRACE_HIT Hit = Cast_Ray(vStart, vEnd, ENUM_CLASS(m_eTargetLayer), eKind);
	if (!Hit.isHit)
		Hit = Cast_Ray(vStart, vEnd, ENUM_CLASS(COLLISIONLAYER::MAP), eKind);
	return Hit;
}

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

// 손 뻗기 대역(머리~최대 리치) 전방 캡슐 캐스트
void CEnvironmentQueryComponent::Scan_Reach()
{
	OBSTACLE_SCAN& Scan = m_Perception.Scan;

	_vector vLook = Get_ScanDir();
	XMStoreFloat3(&Scan.vScanDir, vLook);

	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	_vector vBottom = vCenter - XMVectorSet(0.f, m_pBodyProfile->fHeight * 0.5f, 0.f, 0.f);
	const _float fBandCenterY = (m_pBodyProfile->fHeadHeight + m_pBodyProfile->fMaxReach) * 0.5f;
	_vector vStart = vBottom + XMVectorSet(0.f, fBandCenterY, 0.f, 0.f);

	SHAPE_CAST_HIT Hit{};
	if (!m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), XMQuaternionIdentity(),
			vStart, vLook, m_fLineTraceDistance, ENUM_CLASS(m_eTargetLayer), Hit))
		return;

	Scan.isObstacleDetected		   = true;
	Scan.Reach.Hit.isHit           = true;
	Scan.Reach.Hit.vHitPosition    = _float3(Hit.vHitPoint.x, Hit.vHitPoint.y, Hit.vHitPoint.z);
	Scan.Reach.Hit.vHitNormal      = _float3(Hit.vHitNormal.x, Hit.vHitNormal.y, Hit.vHitNormal.z);
	Scan.Reach.Hit.fCenterDistance = Hit.fFraction * m_fLineTraceDistance;
	Scan.Reach.HitBodyID = Hit.HitBodyID;
	Scan.iHeightFlag |= ENUM_CLASS(HEIGHT_HIT_FLAG::REACH);
	if (Hit.pDesc)
		Scan.Reach.eObjectFlag = static_cast<CALLBACK_CLIENT*>(Hit.pDesc)->eObjectParkourFlag;


	Probe_ReachEdge(vBottom);
}

void CEnvironmentQueryComponent::Probe_ReachEdge(_fvector vBottom)
{
	OBSTACLE_SCAN& Scan = m_Perception.Scan;

	_vector vLook = Get_ScanDir();
	_vector vNormalXZ = XMVectorSetY(XMLoadFloat3(&Scan.Reach.Hit.vHitNormal), 0.f);
	_vector vInward = XMVectorGetX(XMVector3LengthSq(vNormalXZ)) > 1e-4f
		? -XMVector3Normalize(vNormalXZ) : vLook; // 벽 Normal의 반대 방향 Vector를 얻는데, 너무 값이 작을 경우 Look벡터를 사용한다.
	_vector vProbeXZ = XMLoadFloat3(&Scan.Reach.Hit.vHitPosition) + vInward * (m_pBodyProfile->fRadius * 0.5f);
	const _float fTopY = XMVectorGetY(vBottom) + m_pBodyProfile->fMaxReach + m_pBodyProfile->fRadius;
	_vector vDownStart = XMVectorSetY(vProbeXZ, fTopY);
	_vector vDownEnd   = XMVectorSetY(vProbeXZ, Scan.Reach.Hit.vHitPosition.y - 0.1f);

	LINE_TRACE_HIT TopHit = Cast_Ray(vDownStart, vDownEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);
	if (!TopHit.isHit)
		return;

	Scan.Reach.hasEdge     = true;
	Scan.Reach.vEdgePos    = _float3(Scan.Reach.Hit.vHitPosition.x, TopHit.vHitPosition.y, Scan.Reach.Hit.vHitPosition.z);
	Scan.Reach.fEdgeHeight = TopHit.vHitPosition.y - XMVectorGetY(vBottom);
}

void CEnvironmentQueryComponent::Measure_Geometry()
{
	if (m_Perception.Scan.iHeightFlag == 0)
		return;

	MEASURE_FRAME Frame = Make_MeasureFrame();
	Measure_Front(Frame);

	if (!Measure_Top(Frame))
		return;

	Measure_TopWidth(Frame);
	Measure_Depth(Frame);
	Measure_StandPos(Frame);
	Measure_Landing(Frame);

	// Vault 후보(무릎 히트·머리 미스)일 때만 경로·착지 클리어런스 측정
	const _uint iFlag = m_Perception.Scan.iHeightFlag;
	if ((iFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::KNEE)) && !(iFlag & ENUM_CLASS(HEIGHT_HIT_FLAG::HEAD))
	 && m_Perception.Geometry.Top.isReachable && m_Perception.Geometry.hasDepth)
	{
		Measure_PathClearance(Frame);
		Measure_LandingClearance(Frame);
	}
}

CEnvironmentQueryComponent::MEASURE_FRAME CEnvironmentQueryComponent::Make_MeasureFrame() const
{
	MEASURE_FRAME Frame{};
	Frame.fRadius = m_pBodyProfile->fRadius;
	Frame.fTotalHeight = m_pBodyProfile->fHeight;

	_vector vCenter = m_pOwnerTransformCom->Get_State(STATE::POSITION) + m_pOwnerColliderCom->Get_Offset();
	Frame.vBottom = vCenter - XMVectorSet(0.f, Frame.fTotalHeight * 0.5f, 0.f, 0.f);
	Frame.fStartY = XMVectorGetY(Frame.vBottom) + m_pBodyProfile->fMaxReach;
	Frame.vTraversal = Get_ScanDir();

	return Frame;
}

void CEnvironmentQueryComponent::Measure_Front(MEASURE_FRAME& Frame)
{
	const OBSTACLE_SCAN& Scan = m_Perception.Scan;
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	const LINE_TRACE_HIT& TopHit = Scan.HeadHit.isHit ? Scan.HeadHit
		: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.KneeHit;

	const LINE_TRACE_HIT& FrontHit = Scan.KneeHit.isHit ? Scan.KneeHit
		: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.HeadHit;

	if (FrontHit.isHit)
	{
		Geo.Front.hasHit = true;
		Geo.Front.vHitPos = FrontHit.vHitPosition;
		Geo.Front.vNormal = FrontHit.vHitNormal;

		_vector vNormalXZ = XMVectorSetY(XMLoadFloat3(&Geo.Front.vNormal), 0.f);
		if (XMVectorGetX(XMVector3LengthSq(vNormalXZ)) > 1e-4f)
			Frame.vTraversal = -XMVector3Normalize(vNormalXZ);

		Geo.Front.fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&Geo.Front.vHitPos) - m_pOwnerTransformCom->Get_State(STATE::POSITION)));
	}
	XMStoreFloat3(&Geo.vTraversalDir, Frame.vTraversal);

	Frame.vStartXZ = XMLoadFloat3(&TopHit.vHitPosition) + Frame.vTraversal * (Frame.fRadius * 0.5f);
}

_bool CEnvironmentQueryComponent::Measure_Top(MEASURE_FRAME& Frame)
{
	const OBSTACLE_SCAN& Scan = m_Perception.Scan;
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	const LINE_TRACE_HIT& TopHit = Scan.HeadHit.isHit ? Scan.HeadHit
		: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.KneeHit;

	_vector vDownStart = XMVectorSetY(Frame.vStartXZ, Frame.fStartY);
	_vector vDownEnd = XMVectorSetY(Frame.vStartXZ, XMVectorGetY(Frame.vBottom));

	LINE_TRACE_HIT TopDownRay = Cast_Ray(vDownStart, vDownEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);

	if (!TopDownRay.isHit || TopDownRay.vHitPosition.y <= TopHit.vHitPosition.y)
	{
		Geo.Top.isReachable = false;
		Geo.Top.vNormal = _float3(0.f, 1.f, 0.f);   // 무효여도 0벡터로 두지 않음
		Geo.Top.fHeight = m_pBodyProfile->fMaxReach;
		return false;
	}

	Geo.Top.isReachable = true;

	_vector vTopN = XMLoadFloat3(&TopDownRay.vHitNormal);
	if (XMVectorGetX(XMVector3LengthSq(vTopN)) < 1e-6f)
		vTopN = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	XMStoreFloat3(&Geo.Top.vNormal, vTopN);

	Geo.Top.fHeight = TopDownRay.vHitPosition.y - XMVectorGetY(Frame.vBottom);
	Frame.fTopSurfaceY = TopDownRay.vHitPosition.y;

	if (Geo.Front.hasHit)
	{
		const LINE_TRACE_HIT& TopHit = Scan.HeadHit.isHit ? Scan.HeadHit
			: Scan.ChestHit.isHit ? Scan.ChestHit : Scan.KneeHit;
		_vector vTopFrontXZ = XMLoadFloat3(&TopHit.vHitPosition);
		Geo.Top.vEdgePos = _float3(XMVectorGetX(vTopFrontXZ), TopDownRay.vHitPosition.y, XMVectorGetZ(vTopFrontXZ));

		//_vector vFrontXZ = XMLoadFloat3(&Geo.Front.vHitPos);
		//Geo.Top.vEdgePos = _float3(XMVectorGetX(vFrontXZ), TopDownRay.vHitPosition.y, XMVectorGetZ(vFrontXZ));
	}
	else
		Geo.Top.vEdgePos = TopDownRay.vHitPosition;

	return true;
}

void CEnvironmentQueryComponent::Measure_TopWidth(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	_vector vSideAxis = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Frame.vTraversal));
	Geo.fTopWidth = 0.f;

	const _float SideOffsets[2] = { -Frame.fRadius, Frame.fRadius };
	for (_uint i = 0; i < 2; ++i)
	{
		_vector vWSample = Frame.vStartXZ + vSideAxis * SideOffsets[i];
		_vector vWStart  = XMVectorSetY(vWSample, Frame.fStartY);
		_vector vWEnd    = XMVectorSetY(vWSample, Frame.fTopSurfaceY);
		if (Cast_Ray(vWStart, vWEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE).isHit)
			Geo.fTopWidth += Frame.fRadius;
	}
}

void CEnvironmentQueryComponent::Measure_Depth(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	const _float fInset = Frame.fRadius * 0.5f; // 침투 깊이
	const _float fSampleStep = Frame.fRadius;

	_float fLastHit   = 0.f;
	_float fFirstMiss = -1.f;
	for (_uint i = 1; i <= FDEPTH_SAMPLE_COUNT; ++i)
	{
		_float  fDist   = fSampleStep * static_cast<_float>(i);
		_vector vSample = Frame.vStartXZ + Frame.vTraversal * fDist;
		_vector vSStart = XMVectorSetY(vSample, Frame.fStartY);
		_vector vSEnd   = XMVectorSetY(vSample, Frame.fTopSurfaceY);
		if (Cast_Ray(vSStart, vSEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE).isHit)
			fLastHit = fDist;
		else
		{
			fFirstMiss = fDist;
			break;
		}
	}

	if (fFirstMiss < 0.f)
		Geo.fDepth = fInset + fSampleStep * static_cast<_float>(FDEPTH_SAMPLE_COUNT);
	else
		Geo.fDepth = fInset + Refine_DepthEdge(Frame, fLastHit, fFirstMiss);
	Geo.hasDepth = true;
}

_float CEnvironmentQueryComponent::Refine_DepthEdge(const MEASURE_FRAME& Frame, _float fLo, _float fHi)
{
	for (_uint i = 0; i < FEDGE_REFINE_ITERATIONS; ++i)
	{
		_float  fMid    = (fLo + fHi) * 0.5f;
		_vector vSample = Frame.vStartXZ + Frame.vTraversal * fMid;
		_vector vSStart = XMVectorSetY(vSample, Frame.fStartY);
		_vector vSEnd   = XMVectorSetY(vSample, Frame.fTopSurfaceY);
		if (Cast_Ray(vSStart, vSEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::REFINE).isHit)
			fLo = fMid;
		else
			fHi = fMid;
	}
	return (fLo + fHi) * 0.5f;
}

void CEnvironmentQueryComponent::Measure_StandPos(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	if (!Geo.Top.isReachable || !Geo.hasDepth)
		return;

	const _float fStandMargin = 0.15f;
	_float fStandInset = (Geo.fDepth >= 2.f * Frame.fRadius)
		? std::clamp(Frame.fRadius + fStandMargin, Frame.fRadius, Geo.fDepth - Frame.fRadius)
		: Geo.fDepth * 0.5f;

	_vector vStandXZ = XMLoadFloat3(&Geo.Top.vEdgePos) + Frame.vTraversal * fStandInset;
	_vector vStandStart = XMVectorSetY(vStandXZ, Frame.fStartY);
	_vector vStandEnd   = XMVectorSetY(vStandXZ, XMVectorGetY(Frame.vBottom));
	LINE_TRACE_HIT StandHit = Cast_Ray(vStandStart, vStandEnd, ENUM_CLASS(m_eTargetLayer), RAY_KIND::MEASURE);

	if (StandHit.isHit)
		Geo.Top.vStandPos = StandHit.vHitPosition;
	else
		Geo.Top.vStandPos = _float3(XMVectorGetX(vStandXZ), Frame.fTopSurfaceY, XMVectorGetZ(vStandXZ));
}

void CEnvironmentQueryComponent::Measure_Landing(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	_vector vLandXZ    = Frame.vStartXZ + Frame.vTraversal * (Geo.fDepth + Frame.fRadius);
	_vector vLandStart = XMVectorSetY(vLandXZ, Frame.fTopSurfaceY + 0.05f);
	_vector vLandEnd   = XMVectorSetY(vLandXZ, XMVectorGetY(Frame.vBottom) - Frame.fTotalHeight);

	LINE_TRACE_HIT LandRayHit = Cast_Ray_WithMapFallback(vLandStart, vLandEnd, RAY_KIND::MEASURE);

	Geo.Landing.hasSpace = LandRayHit.isHit;
	if (!LandRayHit.isHit)
		return;

	Geo.Landing.vPos = LandRayHit.vHitPosition;

	const _float ProbeOffsets[2] = { -Frame.fRadius * 0.75f, Frame.fRadius * 0.75f };
	for (_uint i = 0; i < 2; ++i)
	{
		_vector vProbeXZ = vLandXZ + Frame.vTraversal * ProbeOffsets[i];
		_vector vPStart  = XMVectorSetY(vProbeXZ, Frame.fTopSurfaceY + 0.05f);
		_vector vPEnd    = XMVectorSetY(vProbeXZ, XMVectorGetY(Frame.vBottom) - Frame.fTotalHeight);

		LINE_TRACE_HIT ProbeHit = Cast_Ray_WithMapFallback(vPStart, vPEnd, RAY_KIND::MEASURE);
		if (!ProbeHit.isHit || fabsf(ProbeHit.vHitPosition.y - Geo.Landing.vPos.y) > FLANDING_MAX_HEIGHT_DIFF)
		{
			Geo.Landing.hasSpace = false;
			break;
		}
	}
}

_bool CEnvironmentQueryComponent::Sweep_BodyCapsule(_fvector vCenter, _fvector vDir, _float fDist, SHAPE_CAST_HIT& OutHit)
{
	// Client는 Jolt 셰이프를 직접 생성할 수 없으므로 엔진이 만든 소유자 바디 캡슐(Get_Shape)을 재사용한다.
	SHAPE_CAST_HIT HitA{}, HitB{};
	OutputDebugStringA("[EQDBG-SRC] Sweep_BodyCapsule\n");
	const _bool isHitA = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), XMQuaternionIdentity(), vCenter, vDir, fDist, ENUM_CLASS(m_eTargetLayer), HitA);
	const _bool isHitB = m_pGameInstance->Shape_Cast(m_pOwnerColliderCom->Get_Shape(), XMQuaternionIdentity(), vCenter, vDir, fDist, ENUM_CLASS(COLLISIONLAYER::MAP), HitB);

	if (isHitA && isHitB) OutHit = (HitA.fFraction <= HitB.fFraction) ? HitA : HitB;
	else if (isHitA)      OutHit = HitA;
	else if (isHitB)      OutHit = HitB;
	return isHitA || isHitB;
}

// 통과 경로 검사
void CEnvironmentQueryComponent::Measure_PathClearance(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;

	const _float fHalfExtent = Frame.fTotalHeight * 0.5f;
	const _float fCenterY = Frame.fTopSurfaceY + FCLEAR_EPS + fHalfExtent;
	const _float fDist    = Geo.Front.fDistance + Geo.fDepth + Frame.fRadius * 2.f; 

	_vector vStart = XMVectorSetY(Frame.vBottom, fCenterY);
	_vector vStandXZ  = XMVectorSetY(XMLoadFloat3(&Geo.Top.vStandPos), fCenterY);
	_float  fStandDist = XMVectorGetX(XMVector3Dot(vStandXZ - vStart, Frame.vTraversal));

	SHAPE_CAST_HIT Hit{};
	const _bool isHit = Sweep_BodyCapsule(vStart, Frame.vTraversal, fDist, Hit);

	// 설 자리 발판 반경
	const _float fStandGate = fStandDist;
	Geo.isPathBlocked  = isHit; // Vault 금지 => 캐릭터~착지 통로 어디든 걸림
	Geo.isStandBlocked = isHit && (Hit.fFraction * fDist <= fStandGate); // Mantle 금지 => 발판 앞에서 걸림

	if (Geo.isPathBlocked || Geo.isStandBlocked)
		Geo.Landing.isBlocked = true;

#ifdef _DEBUG
	if (m_pGameInstance->IsParkourDebug())
	{
		const JPH::Color Color = Geo.isPathBlocked ? JPH::Color(255.f, 0.f, 0.f, 1.f) : JPH::Color(0.f, 255.f, 0.f, 1.f);

		// 캡슐 스윕(Shape Cast) 경로 라인 → ShapeCast 카테고리(2번)
		if (m_pGameInstance->IsDebugShape())
			m_pGameInstance->Add_DebugLine(vStart, vStart + Frame.vTraversal * fDist, Color);

		// 마커 스피어 → DebugSphere 카테고리(3번)
		if (m_pGameInstance->IsDebugSphere())
		{
			// 충돌지점
			m_pGameInstance->Add_DebugSphere(Frame.vBottom, 0.1f, JPH::Color(0.F, 0.F, 255.F, 1.F));

			// 시작 위치?
			m_pGameInstance->Add_DebugSphere(Frame.vBottom, 0.1f, JPH::Color(0.F, 0.F, 255.F, 1.F));
			m_pGameInstance->Add_DebugSphere(XMVectorSetW(XMLoadFloat3(&Geo.Top.vStandPos), 1.f), 0.1f, JPH::Color(0.F, 0.F, 255.F, 1.F));
			m_pGameInstance->Add_DebugSphere(XMVectorSetW(XMLoadFloat3(&Geo.Landing.vPos), 1.f), 0.2f, JPH::Color(0.F, 0.F, 0.F, 1.F));
			if (isHit)
				m_pGameInstance->Add_DebugSphere(XMVectorSetW(XMLoadFloat4(&Hit.vHitPoint), 1.f), 0.1f, JPH::Color(0.F, 0.F, 255.F, 1.F));
		}
	}
#endif
}

void CEnvironmentQueryComponent::Measure_LandingClearance(const MEASURE_FRAME& Frame)
{
	OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	if (!Geo.Landing.hasSpace)
		return; // 착지 지점 자체가 없음(낭떠러지)

	Geo.Landing.isElevated = (Geo.Landing.vPos.y - XMVectorGetY(Frame.vBottom)) > FLANDING_ELEVATED_THRESHOLD;

	const _float fHalfExtent    = Frame.fTotalHeight * 0.5f;
	const _float fStartBottomY  = Frame.fTopSurfaceY + FCLEAR_EPS;
	const _float fSweepDist     = fStartBottomY - Geo.Landing.vPos.y;
	if (fSweepDist <= 0.f)
		return; // 착지면이 상단면보다 높은 이상 케이스
}


#ifdef _DEBUG
void CEnvironmentQueryComponent::Draw_DebugMarkers()
{
	if (!m_pGameInstance->IsParkourDebug() || !m_pGameInstance->IsDebugSphere())
		return;

	const OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	if (Geo.Top.isReachable)
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&Geo.Top.vEdgePos), 0.07f, JPH::Color(255.f, 0.f, 0.f, 1.f));
	if (Geo.Landing.hasSpace)
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&Geo.Landing.vPos), 0.07f, JPH::Color(255.f, 255.f, 0.f, 1.f));
}

void CEnvironmentQueryComponent::Log_ShapeHit(const SHAPE_CAST_HIT& Hit)
{
	if (Hit.pDesc == m_pDebugLastShapeHitDesc)
		return;

	m_pDebugLastShapeHitDesc = Hit.pDesc;
	cout << "[EnvQuery] ShapeCast hit  pos=(" << Hit.vHitPoint.x << ", "
	     << Hit.vHitPoint.y << ", " << Hit.vHitPoint.z
	     << ")  flag=" << ENUM_CLASS(static_cast<CALLBACK_CLIENT*>(Hit.pDesc)->eObjectParkourFlag) << endl;
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

	cout << "[Geometry] H=" << Geo.Top.fHeight
	     << " D=" << Geo.fDepth
	     << " W=" << Geo.fTopWidth
	     << " Front=" << Geo.Front.fDistance
	     << " Top=" << (Geo.Top.isReachable ? "O" : "X")
	     << " Landing=" << (Geo.Landing.hasSpace ? "O" : "X") << endl;
}
#endif // _DEBUG

_bool CEnvironmentQueryComponent::Find_Ground(_fvector vProbePos, _float fUpOffset, _float fMaxDrop, _float3& vOutGroundPos)
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

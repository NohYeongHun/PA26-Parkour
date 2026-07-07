#include "EnginePch.h"
#include "PhysicsManager.h"

#include "ContactListenerImpl.h"
#include "CharacterContactListenerImpl.h"

#include "Rigidbody.h"
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "GameInstance.h"

CPhysicsManager::CPhysicsManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }, m_pContext{ pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CPhysicsManager::IsChangeLevel(_bool isChangeLevel)
{
	m_pContactListener->IsChangeLevel(isChangeLevel);
}

Body* CPhysicsManager::Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut)
{
	Body* body = m_pPhysicsSystem->GetBodyInterface().CreateBody(BodySetting);
	ASSERT_CRASH(body);

	m_pPhysicsSystem->GetBodyInterface().AddBody(body->GetID(), EActivation::Activate);

	*pOut = &m_pPhysicsSystem->GetBodyInterface();

	return body;
}

Character* CPhysicsManager::Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	return new Character(&CharacterSetting, vPos, vQuat, reinterpret_cast<JPH::uint64>(pUserData), m_pPhysicsSystem);
}

Ref<CharacterVirtual> CPhysicsManager::Register_CharacterVirtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData, BodyInterface** pOut)
{
	Ref<CharacterVirtual> pInstance = new CharacterVirtual(&CharacterSetting, vPos, vQuat, reinterpret_cast<JPH::uint64>(pUserData), m_pPhysicsSystem);
	ASSERT_CRASH(pInstance);

	// Character VS Character Collision SetUp
	pInstance->SetCharacterVsCharacterCollision(m_pCVCCollision);
	// Chararcter VS Character Collision???깅줉
	m_pCVCCollision->Add(pInstance);
	// CharacterContactListener SetUp
	pInstance->SetListener(m_pCharacterContactListener);

	if (nullptr != pOut)
		*pOut = &m_pPhysicsSystem->GetBodyInterface();

	return pInstance;
}

void CPhysicsManager::Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer)
{
	ASSERT_CRASH(pVirtual);

	m_Virtuals[iObjectLayer].push_back(pVirtual);
}

void CPhysicsManager::Register_Virtual(CharacterVirtual* pVirtual)
{
	m_pCVCCollision->Add(pVirtual);
}

void CPhysicsManager::Remove_Virtual(CharacterVirtual* pVirtual)
{
	m_pCVCCollision->Remove(pVirtual);
}

void CPhysicsManager::Clear_Resource()
{
	m_pContactListener->Clear_Resource();
}

HRESULT CPhysicsManager::Initialize(_uint iNumObjectLayer)
{
	ASSERT_CRASH(iNumObjectLayer > 0);

	m_iNumObjectLayer = iNumObjectLayer;

	// Register Allocator
	RegisterDefaultAllocator();
	// Create factory
	Factory::sInstance = new Factory;
	// Register physics types with the factory
	RegisterTypes();

	m_pAllocator = new TempAllocatorImpl(32 * 1024 * 1024);
	ASSERT_CRASH(m_pAllocator);
	// Thread Pool
	m_pJobSystem = new JobSystemThreadPool(2048, 8, m_iMaxJob - 1);
	ASSERT_CRASH(m_pJobSystem);

	// Layer ?앹꽦
	m_pBPLayer = new BPLayer(iNumObjectLayer);
	ASSERT_CRASH(m_pBPLayer);
	// Filter ?앹꽦
	m_pObjectLayerFilter = new ObjectLayerPairFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectLayerFilter);
	m_pObjectVsBPFilter = new ObjectVsBroadPhaseLayerFilterImpl(iNumObjectLayer);
	ASSERT_CRASH(m_pObjectVsBPFilter);

	// Virtual Container Create
	m_Virtuals = new vector<CharacterVirtual*>[m_iNumObjectLayer];
	// CharacterVirtual VS CharacterVirtual Collision
	m_pCVCCollision = new CharacterVsCharacterCollisionSimple();

	// RayFilter
	m_pRayFilter = new SpecifiedBroadPhaseLayerFilter(static_cast<BroadPhaseLayer>(ENUM_CLASS(BPLAYER::NON_MOVE)));

#ifdef _DEBUG
	m_pDebugRenderer = new CDebugRender(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pDebugRenderer);

	m_DrawSetting.mDrawShape = true;
	m_DrawSetting.mDrawShapeWireframe = false;
#endif

	m_ExtendedUpdateSetting.mStickToFloorStepDown = { 0.f, -0.35f, 0.f };					// 바닥에 붙이기 (아래로 Ray 쏴서)
	m_ExtendedUpdateSetting.mWalkStairsStepUp = Vec3{ 0.f, 0.50f, 0.f };					// 계단 허용 높이
	m_ExtendedUpdateSetting.mWalkStairsStepDownExtra = Vec3{ 0.f, -0.10f, 0.f };		// 아래로 내려갈 때 여유 (붕 뜨는 느낌 방지)
	m_ExtendedUpdateSetting.mWalkStairsMinStepForward = 0.28f;							// 단차 이동 시, 최소 필요 이동 거리
	m_ExtendedUpdateSetting.mWalkStairsStepForwardTest = 0.55f;							// 단차 이동 시, 앞으로 Ray 쏴서 올라갈 수 있는가 Test
	m_ExtendedUpdateSetting.mWalkStairsCosAngleForwardContact = cosf(XMConvertToRadians(75.f));			// 각도 허용치
	
	return S_OK;
}

void CPhysicsManager::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_DELETE) == KEYSTATE::DOWN)
		m_isRenderAll = !m_isRenderAll;

	if (m_pGameInstance->Get_DIKeyState(DIK_END) == KEYSTATE::DOWN)
		m_isParkourDebug = !m_isParkourDebug;
#endif
	m_pContactListener->Clear_Resource();
	m_pPhysicsSystem->Update(fTimeDelta, 1, m_pAllocator, m_pJobSystem);

	for (_uint i = 0; i < m_iNumObjectLayer; ++i)
	{
		for (auto& pVirtual : m_Virtuals[i])
		{
			// BroadPhaseLayerFilter
			DefaultBroadPhaseLayerFilter BPLayerFilter = DefaultBroadPhaseLayerFilter(*m_pObjectVsBPFilter, ObjectLayer(i));
			DefaultObjectLayerFilter ObjectLayerFilter = DefaultObjectLayerFilter(*m_pObjectLayerFilter, ObjectLayer(i));
			BodyFilter bodyFilter = BodyFilter();
			ShapeFilter shapeFilter = ShapeFilter();

			pVirtual->ExtendedUpdate(fTimeDelta, Vec3(0.f, -9.81f, 0.f),
				m_ExtendedUpdateSetting,
				BPLayerFilter,
				ObjectLayerFilter,
				bodyFilter,
				shapeFilter,
				*m_pAllocator
			);

			pVirtual = nullptr;
		}
		m_Virtuals[i].clear();
	}
}

void CPhysicsManager::Late_Update()
{
	if (nullptr == m_pContactListener)
		return;
	m_pContactListener->Remove_Update();
}

_bool CPhysicsManager::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, _float4* pOut)
{
	RVec3 StartPos = LoadVec3(vStartPos);
	RVec3 EndPos = LoadVec3(vEndPos);

#ifdef _DEBUG
	_float3 vSP{}, vEP{};
	XMStoreFloat3(&vSP, vStartPos);
	XMStoreFloat3(&vEP, vEndPos);
	m_RayPoint.push_back(make_pair(vSP, vEP));
#endif

	_vector vDir = vEndPos - vStartPos;

	RRayCast ray(StartPos, (EndPos - StartPos));
	RayCastResult result;

	_float fOriginFraction = result.mFraction;
	m_pPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result, *m_pRayFilter);

	if (nullptr != pOut)
	{
		_float fDistanceOffset = 0.8f;
		if (result.mFraction < 0.1f)
			return false;
		XMStoreFloat4(pOut, vStartPos + result.mFraction * vDir * fDistanceOffset);
	}

	return fOriginFraction > result.mFraction && result.mFraction > 0.f ? true : false;
}

_bool CPhysicsManager::Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, const uint16 iTargetObjectLayer, _float4* pOut)
{
	RVec3 StartPos = LoadVec3(vStartPos);
	RVec3 EndPos = LoadVec3(vEndPos);

#ifdef _DEBUG
	_float3 vSP{}, vEP{};
	XMStoreFloat3(&vSP, vStartPos);
	XMStoreFloat3(&vEP, vEndPos);
	m_RayPoint.push_back(make_pair(vSP, vEP));
#endif

	_vector vDir = vEndPos - vStartPos;

	RRayCast ray(StartPos, (EndPos - StartPos));
	RayCastResult result;

	_float fOriginFraction = result.mFraction;
	SpecifiedObjectLayerFilter ObjectLayerFilter{ ObjectLayer{iTargetObjectLayer} };

	m_pPhysicsSystem->GetNarrowPhaseQuery().CastRay(ray, result, *m_pRayFilter, ObjectLayerFilter);

	if (nullptr != pOut)
	{
		_float fDistanceOffset = 0.8f;
		if (result.mFraction < 0.1f)
			return false;
		XMStoreFloat4(pOut, vStartPos + result.mFraction * vDir * fDistanceOffset);
	}

	return fOriginFraction > result.mFraction && result.mFraction > 0.f ? true : false;
}

_bool CPhysicsManager::Box_Cast(const CRigidbody* pRigidbodyCom, const _fvector& vDir, const _float fDistance, const uint16 iObjectLayer, vector<BOX_CAST_HIT>& OutHits)
{
	if (nullptr == pRigidbodyCom)
		return false;

	const Body* pBody = pRigidbodyCom->Get_Body();
	if (nullptr == pBody)
		return false;

	OutHits.clear();
	RMat44 StartTransform = pBody->GetWorldTransform();

	// 방향
	Vec3 Direction = LoadVec3(vDir).Normalized() * fDistance;
	// ShapeCast 객체
	RefConst<Shape> pShape = pBody->GetShape();
	RShapeCast ShapeCastIn = RShapeCast::sFromWorldTransform(
		pShape, Vec3::sReplicate(1.f), StartTransform, Direction
	);

	ShapeCastSettings CastSettings;

	SpecifiedObjectLayerFilter ObjectLayerFilter{ ObjectLayer{iObjectLayer} };

	// 충돌한 객체 후보군을 모을 곳
	AllHitCollisionCollector<CastShapeCollector> Collector;
	m_pPhysicsSystem->GetNarrowPhaseQuery().CastShape(
		ShapeCastIn, CastSettings, RVec3::sZero(), Collector,
		*m_pRayFilter, ObjectLayerFilter);

	_bool isHit = Collector.HadHit();
	_float fEndFraction = 1.f;

#ifdef _DEBUG
	vector<_float3> HitPointsDebug;
#endif

	if (isHit)
	{
		Collector.Sort();
		fEndFraction = Collector.mHits[0].mFraction;

		OutHits.reserve(OutHits.size() + Collector.mHits.size());
		for (const ShapeCastResult& Hit : Collector.mHits)
		{
			BOX_CAST_HIT Result{};
			Result.fFraction = Hit.mFraction;
			Result.HitBodyID = Hit.mBodyID2;
			XMStoreFloat4(&Result.vHitPoint,
				XMVectorSet(Hit.mContactPointOn2.GetX(), Hit.mContactPointOn2.GetY(), Hit.mContactPointOn2.GetZ(), 1.f));
			XMStoreFloat4(&Result.vHitNormal,
				XMVectorSet(Hit.mPenetrationAxis.GetX(), Hit.mPenetrationAxis.GetY(), Hit.mPenetrationAxis.GetZ(), 0.f));

			OutHits.push_back(Result);

#ifdef _DEBUG
			HitPointsDebug.push_back(_float3(Result.vHitPoint.x, Result.vHitPoint.y, Result.vHitPoint.z));
#endif
		}
	}

#ifdef _DEBUG
	{
		BOX_CAST_DEBUG Debug{};
		Debug.pShape = pShape;
		Debug.StartMatrix = StartTransform;
		Debug.EndMatrix = StartTransform.PostTranslated(Direction * fEndFraction);
		Debug.isHit = isHit;
		Debug.HitPoints = move(HitPointsDebug);
		m_BoxCastDebugs.push_back(move(Debug));
	}
#endif

	return isHit;
}

_bool CPhysicsManager::Shape_Cast(RefConst<Shape> pShape, const _fvector& vQuat, const _fvector& vPos, const _fvector& vDir, const _float fDistance, const uint16 iTargetObjectLayer, SHAPE_CAST_HIT& OutHit)
{
	// 1. 제약 조건
	if (nullptr == pShape)
		return false;

	// 2. 현재 시점의 Rot, Translation
	JPH::RVec3 startPos = LoadVec3(vPos);
	JPH::Quat  startRot = LoadQuat(vQuat);
	RMat44 StartTransform = Mat44::sRotationTranslation(startRot, startPos);

	// 3. 방향 결정
	Vec3 Direction = LoadVec3(vDir).Normalized() * fDistance;
	RShapeCast ShapeCastIn = RShapeCast::sFromWorldTransform(
		pShape, Vec3::sReplicate(1.f), StartTransform, Direction
	);

	ShapeCastSettings CastSettings;

	// 4. Object Layer Filter
	// => Parkour Layer만 충돌 되도록
	SpecifiedObjectLayerFilter ObjectLayerFilter{ ObjectLayer{iTargetObjectLayer} };

	// ClosestCollector => 가장 가까운 하나의 Shape만 충돌 처리.
	ClosestHitCollisionCollector<CastShapeCollector> Collector;
	m_pPhysicsSystem->GetNarrowPhaseQuery().CastShape(
		ShapeCastIn, CastSettings, RVec3::sZero(), Collector,
		*m_pRayFilter, ObjectLayerFilter);

	_bool isHit = Collector.HadHit();
	_float fEndFraction = 1.f;
#ifdef _DEBUG
	vector<_float3> HitPointsDebug;
#endif
	if (isHit)
	{
		fEndFraction = Collector.mHit.mFraction;
		const ShapeCastResult& Hit = Collector.mHit;
		
		OutHit.fFraction = Hit.mFraction;
		OutHit.HitBodyID = Hit.mBodyID2;
		XMStoreFloat4(&OutHit.vHitPoint,
			XMVectorSet(Hit.mContactPointOn2.GetX(), Hit.mContactPointOn2.GetY(), Hit.mContactPointOn2.GetZ(), 1.f));
		XMStoreFloat4(&OutHit.vHitNormal,
			XMVectorSet(Hit.mPenetrationAxis.GetX(), Hit.mPenetrationAxis.GetY(), Hit.mPenetrationAxis.GetZ(), 0.f));
			
		// Memory Pointer 주소
		uint64 UserData = m_pPhysicsSystem->GetBodyInterface().GetUserData(Hit.mBodyID2);
		if (0 != UserData)
			OutHit.pDesc = reinterpret_cast<COLLISION_DATA*>(UserData)->pDesc;

#ifdef _DEBUG
		HitPointsDebug.push_back(_float3(OutHit.vHitPoint.x, OutHit.vHitPoint.y, OutHit.vHitPoint.z));
#endif // _DEBUG

	}

	

#ifdef _DEBUG
	{
		SHAPE_CAST_DEBUG Debug{};
		Debug.pShape = pShape;
		Debug.StartMatrix = StartTransform; // 시작 지점 행렬
		Debug.EndMatrix = StartTransform.PostTranslated(Direction * fEndFraction); // 끝 지점 행렬
		Debug.isHit = isHit;
		Debug.HitPoints = move(HitPointsDebug);
		m_ShapeCastDebugs.push_back(move(Debug));
	}
#endif
	OutHit.isHit = isHit;

	return isHit;
}

// 현재 시점을 정확하게 구할려면




#ifdef _DEBUG 
// => Debug 용도 Render 함수들.
void CPhysicsManager::Render()
{
	// Ray Render
	for (size_t i = 0; i < m_RayPoint.size(); ++i)
		DrawRay(XMLoadFloat3(&m_RayPoint[i].first), XMLoadFloat3(&m_RayPoint[i].second));
	m_RayPoint.clear();

	// Box Cast Render (파쿠르 디버그 토글에 종속) END키를 켰을때만?
	if (true == m_isParkourDebug)
	{
		for (const SHAPE_CAST_DEBUG& Debug : m_ShapeCastDebugs)
			DrawShapeCast(Debug.pShape, Debug.StartMatrix, Debug.EndMatrix, Debug.isHit, Debug.HitPoints);
	}
	
	m_ShapeCastDebugs.clear();

	if (false == m_isRenderAll)
		return;

	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	m_pPhysicsSystem->DrawBodies(m_DrawSetting, m_pDebugRenderer);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawShape(const Shape* pShape, RMat44 Matrix, Color BodyColor)
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	pShape->Draw(m_pDebugRenderer, Matrix, Vec3(1.f, 1.f, 1.f), BodyColor, false, true);
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawRay(const _fvector& vStartPos, const _fvector& vEndPos)
{
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	m_pDebugRenderer->DrawLine(LoadVec3(vStartPos), LoadVec3(vEndPos), Color(255.f, 0.f, 0.f, 1.f));
	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawBoxCast(const Shape* pShape, const RMat44& StartMatrix, const RMat44& EndMatrix, _bool isHit, const vector<_float3>& HitPoints)
{
	if (nullptr == pShape)
		return;

	Color EndColor = isHit ? Color(255.f, 0.f, 0.f, 1.f) : Color(0.f, 255.f, 0.f, 1.f);

	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();

	// 시작 포즈 (흰색 와이어프레임)
	pShape->Draw(m_pDebugRenderer, StartMatrix, Vec3(1.f, 1.f, 1.f), Color(255.f, 255.f, 255.f, 1.f), false, true);
	// 스윕 종료 포즈 (히트 시 빨강, 아니면 초록)
	pShape->Draw(m_pDebugRenderer, EndMatrix, Vec3(1.f, 1.f, 1.f), EndColor, false, true);
	// 스윕 경로 (노란 화살표)
	m_pDebugRenderer->DrawArrow(StartMatrix.GetTranslation(), EndMatrix.GetTranslation(), Color(255.f, 255.f, 0.f, 1.f), 0.05f);

	// 히트 지점 마커
	for (const _float3& vPoint : HitPoints)
		m_pDebugRenderer->DrawMarker(LoadVec3(vPoint), Color(255.f, 0.f, 0.f, 1.f), 0.15f);

	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
void CPhysicsManager::DrawShapeCast(const Shape* pShape, const RMat44& StartMatrix, const RMat44& EndMatrix, _bool isHit, const vector<_float3>& HitPoints)
{
	if (nullptr == pShape)
		return;

	Color EndColor = isHit ? Color(255.f, 0.f, 0.f, 1.f) : Color(0.f, 255.f, 0.f, 1.f);
	static_cast<CDebugRender*>(m_pDebugRenderer)->Begin();
	pShape->Draw(m_pDebugRenderer, StartMatrix, Vec3(1.f, 1.f, 1.f), Color(255.f, 255.f, 255.f, 1.f), false, true);
	// 스윕 종료 포즈 (히트 시 빨강, 아니면 초록)
	pShape->Draw(m_pDebugRenderer, EndMatrix, Vec3(1.f, 1.f, 1.f), EndColor, false, true);

	// 스윕 경로 (바닥 / 중앙 / 최상단 3갈래 노란 화살표)
	AABox LocalBounds = pShape->GetLocalBounds();
	Vec3 vCenterLocal = LocalBounds.GetCenter();
	Vec3 vBottomLocal = vCenterLocal;
	vBottomLocal.SetY(LocalBounds.mMin.GetY());
	Vec3 vTopLocal = vCenterLocal;
	vTopLocal.SetY(LocalBounds.mMax.GetY());

	Color ArrowColor = Color(255.f, 255.f, 0.f, 1.f);
	m_pDebugRenderer->DrawArrow(StartMatrix * vBottomLocal, EndMatrix * vBottomLocal, ArrowColor, 0.05f);
	m_pDebugRenderer->DrawArrow(StartMatrix * vCenterLocal, EndMatrix * vCenterLocal, ArrowColor, 0.05f);
	m_pDebugRenderer->DrawArrow(StartMatrix * vTopLocal, EndMatrix * vTopLocal, ArrowColor, 0.05f);

	// 히트 지점 마커
	for (const _float3& vPoint : HitPoints)
		m_pDebugRenderer->DrawMarker(LoadVec3(vPoint), Color(255.f, 255.f, 0.f, 1.f), 0.15f);

	static_cast<CDebugRender*>(m_pDebugRenderer)->End();
}
#endif

void CPhysicsManager::SetUp_PhysicsSystem()
{
	// PhysicsSystem ?앹꽦
	m_pPhysicsSystem = new PhysicsSystem();
	m_pPhysicsSystem->Init(
		m_iNumBodies, m_iNumBodyMutexes, 
		m_iMaxBodyPairs, m_iMaxContactConstraints,
		*m_pBPLayer, *m_pObjectVsBPFilter, *m_pObjectLayerFilter);
	m_pPhysicsSystem->SetPhysicsSettings(m_PhysicsSetting);

	// Contact Listener Create / SetUp
	m_pContactListener = new CContactListenerImpl(&m_pPhysicsSystem->GetBodyInterface());
	ASSERT_CRASH(m_pContactListener);
	m_pPhysicsSystem->SetContactListener(m_pContactListener);

	// Character Contact Listener
	m_pCharacterContactListener = new CharacterContactListenerImpl(&m_pPhysicsSystem->GetBodyInterface());

	Vec3 vGravity = Vec3(0, -9.81f, 0);
	m_pPhysicsSystem->SetGravity(vGravity);
}

CPhysicsManager* CPhysicsManager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumObjectLayer)
{
	CPhysicsManager* pInstance = new CPhysicsManager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iNumObjectLayer)))
	{
		MSG_BOX("Failed to Created : PhysicsManager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPhysicsManager::Free()
{
	__super::Free();

	Safe_Delete_Array(m_Virtuals);

	Safe_Delete(m_pBPLayer);
	Safe_Delete(m_pObjectLayerFilter);
	Safe_Delete(m_pObjectVsBPFilter);
	Safe_Delete(m_pRayFilter);
	Safe_Delete(m_pCVCCollision);
#ifdef _DEBUG
	Safe_Delete(m_pDebugRenderer);
#endif
	Safe_Delete(m_pPhysicsSystem);
	Safe_Delete(m_pCharacterContactListener);
	Safe_Delete(m_pContactListener);
	Safe_Delete(m_pJobSystem);
	Safe_Delete(m_pAllocator);
	Safe_Delete(Factory::sInstance);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}

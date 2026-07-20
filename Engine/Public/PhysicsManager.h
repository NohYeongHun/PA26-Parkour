#pragma once
#include "Base.h"

#include "CollisionLayer.h"
#include "DebugRender.h"

NS_BEGIN(JPH)
class TempAllocator;
class JobSystem;
class PhysicsSystem;
NS_END

NS_BEGIN(Engine)

class CPhysicsManager final : public CBase
{
private:
	explicit CPhysicsManager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPhysicsManager() = default;

public:
	void				IsChangeLevel(_bool isChangeLevel);

public:
#pragma region Init
	// Physics System 세팅
	void				SetUp_PhysicsSystem();
	// Object -> BroadPhase 맵핑
	void				SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer) {
		ASSERT_CRASH(nullptr != m_pBPLayer);
		m_pBPLayer->SetUp_ObjectToBP(iObjectLayer, iBPLayer);
	};
	// Object VS Object Layer Setting
	void				SetUp_ObjectFilter(_uint iSrc, _uint iDst) {
		ASSERT_CRASH(nullptr != m_pObjectLayerFilter);
		m_pObjectLayerFilter->SetUp_ObjectFilter(iSrc, iDst);
	};
	// Object VS BroadPhase Layer Setting
	void				SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer) {
		ASSERT_CRASH(nullptr != m_pObjectVsBPFilter);
		m_pObjectVsBPFilter->SetUp_ObjectVsBPFilter(iObjectLayer, iBPLayer);
	};
#pragma endregion

	// Body 생성
	Body*					Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut);
	// Character 생성
	Character*			Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData);
	// CharacterVirtual 생성
	Ref<CharacterVirtual>	Register_CharacterVirtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData, BodyInterface** pOut);

	void					Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer);
	void					Register_Virtual(CharacterVirtual* pVirtual);
	void					Remove_Virtual(CharacterVirtual* pVirtual);

	void					Clear_Resource();

public:
	HRESULT				Initialize(_uint iNumObjectLayer);
	void				Update(_float fTimeDelta);
	void				Late_Update();
#ifdef _DEBUG
	void				Render();
	void				DrawShape(const Shape* pShape, RMat44 Matrix, Color BodyColor = Color(0.f, 255.f, 0.f, 1.f));
	void				DrawRay(const _fvector& vStartPos, const _fvector& vEndPos);
	void				DrawBoxCast(const Shape* pShape, const RMat44& StartMatrix, const RMat44& EndMatrix, _bool isHit, const vector<_float3>& HitPoints);

	void				DrawShapeCast(const Shape* pShape, const RMat44& StartMatrix, const RMat44& EndMatrix, _bool isHit, const vector<_float3>& HitPoints);
	_bool				IsParkourDebug() { return m_isParkourDebug; }

	// 지정한 지점에 와이어 스피어
	void				Add_DebugSphere(const _fvector& vCenter, _float fRadius, const Color& color = Color(0.f, 255.f, 255.f, 1.f));
	// 지정한 두 지점 사이의 라인
	void				Add_DebugLine(const _fvector& vStart, const _fvector& vEnd, const Color& color = Color(0.f, 255.f, 0.f, 1.f));
#endif

	_bool				Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, _float4* pOut);
	_bool				Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, const uint16 iTargetObjectLayer, _float4* pOut);
	RAY_CAST_HIT		Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, const uint16 iTargetObjectLayer);
	_bool				Shape_Cast(RefConst<Shape> pShape,  const _fvector& vQuat, const _fvector& vPos, const _fvector& vDir, const _float fDistance, const uint16 iTargetObjectLayer, SHAPE_CAST_HIT& OutHit);
	// 스피어 스윕
	_bool				Sphere_Cast(const _fvector& vPos, const _fvector& vDir, const _float fDistance, const _float fRadius, const uint16 iTargetObjectLayer, SHAPE_CAST_HIT& OutHit);
	_bool				Get_Body_AABB(const BodyID& ID, _float3& vOutMin, _float3& vOutMax);
	RefConst<Shape> Get_SphereShape(_float fRadius);
	

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	TempAllocator*		m_pAllocator = { nullptr };
	JobSystem*			m_pJobSystem = { nullptr };
	PhysicsSystem*		m_pPhysicsSystem = { nullptr };
	class CContactListenerImpl*	m_pContactListener = { nullptr };

	PhysicsSettings		m_PhysicsSetting;
	CharacterVirtual::ExtendedUpdateSettings m_ExtendedUpdateSetting;

	BPLayer*											m_pBPLayer = { nullptr };
	ObjectLayerPairFilterImpl*					m_pObjectLayerFilter = { nullptr };
	ObjectVsBroadPhaseLayerFilterImpl*		m_pObjectVsBPFilter = { nullptr };

	CharacterVsCharacterCollisionSimple*	m_pCVCCollision = { nullptr };
	CharacterContactListener*					m_pCharacterContactListener = { nullptr };
	SpecifiedBroadPhaseLayerFilter*			m_pRayFilter = { nullptr };

	_uint		m_iNumBodies = { 10240 };
	_uint		m_iNumBodyMutexes = {}; // Autodetect
	_uint		m_iMaxBodyPairs = { 65536 };
	_uint		m_iMaxContactConstraints = { 20480 };

	_uint		m_iMaxJob = { thread::hardware_concurrency() };

	vector<CharacterVirtual*>*				m_Virtuals = { nullptr };
	_uint		m_iNumObjectLayer = {};

#ifdef _DEBUG
	DebugRenderer*	m_pDebugRenderer = { nullptr };
	BodyManager::DrawSettings m_DrawSetting;
	_bool m_isRenderAll = { false };
	_bool m_isParkourDebug = { false };
	_uint m_iHighlightLayer = {};
	vector<pair<_float3, _float3>>	m_RayPoint;

	unordered_map<_float, RefConst<Shape>> m_SphereShapeCache;


	struct BOX_CAST_DEBUG
	{
		const Shape*			pShape;
		RMat44					StartMatrix;
		RMat44					EndMatrix;
		_bool					isHit;
		vector<_float3>			HitPoints;
	};

	struct SHAPE_CAST_DEBUG
	{
		RefConst<Shape>			pShape; // 임시 셰이프(Sphere_Cast)도 렌더 시점까지 살아있도록 참조 보관 (lifetime)
		RMat44					StartMatrix;
		RMat44					EndMatrix;
		_bool					isHit;
		vector<_float3>			HitPoints;
	};


	struct DEBUG_SPHERE
	{
		_float3	vCenter;
		_float	fRadius;
		Color	color;
	};

	struct DEBUG_LINE
	{
		_float3	vStart;
		_float3	vEnd;
		Color	color;
	};

	vector<BOX_CAST_DEBUG>	m_BoxCastDebugs;
	vector<SHAPE_CAST_DEBUG> m_ShapeCastDebugs;
	vector<DEBUG_SPHERE>	m_DebugSpheres;
	vector<DEBUG_LINE>		m_DebugLines;
#endif

public:
	static CPhysicsManager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumObjectLayer);
	virtual void Free() override;
};

NS_END
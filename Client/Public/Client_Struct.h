#pragma once
#include "Engine_Define.h"
#include "Client_Enum.h"

namespace Engine
{
	class CTransform;
}

namespace Client
{
	
#pragma region CHARACTER
	typedef struct SLIDE_DATA
	{
		vector<_float3> WayPoints;
		_float fSpeed = 5.f;

		void Reset()
		{
			WayPoints.clear();
		}
	}SLIDE_DATA;

	typedef struct tagGrappleInfo {
		CTransform* pTransform = { nullptr };
		OBJECTTYPE eObjectType;
		uint* pTriggerIndex = { nullptr };
		_bool IsActive = { false };
		void Reset()
		{
			pTransform = nullptr;
			eObjectType = OBJECTTYPE::END;
			pTriggerIndex = { nullptr };
			IsActive = { false };
		}
	}GRAPPLE_INFO;
#pragma endregion

	typedef struct tagTargetInfo
	{
		CTransform* pTransform = nullptr;
		const _float4x4* pSocketMatrix = nullptr;
		_bool IsActive = { false };
		// 초기화
		void Reset() { pTransform = nullptr; pSocketMatrix = nullptr; IsActive = false; }

		_bool operator== (const tagTargetInfo& target) const
		{
			return this->pTransform == target.pTransform && this->pSocketMatrix == target.pSocketMatrix;
		}
	}TARGET_INFO;


	typedef struct tagCallBackClientDesc
	{
		void* pTransform = { nullptr };  // Transform;
		OBJECTTYPE eObjectType{ OBJECTTYPE::END }; 
		PARKOUR_FLAG eObjectParkourFlag = { PARKOUR_FLAG::END };
	}CALLBACK_CLIENT;


#pragma region ENVIRONMENT
	// 캐릭터 체형 프로필, 초기화 시 콜라이더에서 1회 추출
	typedef struct tagBodyProfile
	{
		_float fHeight = 0.f;      
		_float fRadius = 0.f;      
		_float fKneeHeight = 0.f;  
		_float fChestHeight = 0.f; 
		_float fHeadHeight = 0.f;  
		_float fMaxReach = 0.f;    
	}BODY_PROFILE;

	typedef struct tagLineTraceHit
	{
		_bool		isHit = { false };
		_float      fCenterDistance = 0.f;
		_float3     vHitPosition{};
		_float3     vHitNormal{};
	}LINE_TRACE_HIT;

	enum class HEIGHT_HIT_FLAG : _uint { KNEE = 1 << 0, CHEST = 1 << 1, HEAD = 1 << 2, REACH = 1 << 3 };

	typedef struct tagReachScan {
		LINE_TRACE_HIT Hit;
		PARKOUR_FLAG   eObjectFlag = PARKOUR_FLAG::END;
		BodyID         HitBodyID{};
		_bool          hasEdge = false;
		_float3        vEdgePos{};
		_float         fEdgeHeight = 0.f;
	}REACH_SCAN;

	typedef struct tagObstacleScan {
		_bool          isObstacleDetected = false;
		PARKOUR_FLAG   eObjectFlag = PARKOUR_FLAG::END;
		BodyID         HitBodyID{}; // 기본 장애물 Hit Id
		_float3        vScanDir{};
		LINE_TRACE_HIT KneeHit;
		LINE_TRACE_HIT ChestHit;
		LINE_TRACE_HIT HeadHit;
		LINE_TRACE_HIT LeftChestHit;
		LINE_TRACE_HIT RightChestHit;
		_uint          iHeightFlag = 0;
		REACH_SCAN     Reach;
	}OBSTACLE_SCAN;

	typedef struct tagActionVerdict {
		_bool         isPossible = false;
		REJECT_REASON eReject = REJECT_REASON::NONE;
	}ACTION_VERDICT;

	typedef struct tagParkourDecision {
		ACTION_VERDICT Verdicts[ENUM_CLASS(PARKOUR_ACTION::END)];
		_uint          iCandidateFlag = 0;
		_float         fApproachDot = 0.f;
		PARKOUR_ACTION eBestEnvAction = PARKOUR_ACTION::NONE;
		_bool          isValid = false;
		_bool          isTopReached = false;
		PARKOUR_ACTION eCommand = PARKOUR_ACTION::NONE;
		_bool isGrounded  = false;
		_bool isSupported = false;
		_bool isFalling   = false;
		_bool hasMoveInput  = false;
		_bool wantsRun      = false;
		_bool wantsJump     = false;
		_bool wantsForward  = false;
		_bool wantsDown     = false;
		_bool wantsLeft      = false; 
		_bool wantsRight     = false; 
		_bool wantsJumpPress = false; 
	}PARKOUR_DECISION;

	typedef struct tagGeoFront {
		_bool   hasHit = false;
		_float3 vHitPos{};
		_float3 vNormal{};
		_float  fDistance = 0.f;
	}GEO_FRONT;

	typedef struct tagGeoTop {
		_bool   isReachable = false;
		_float3 vEdgePos{};
		_float3 vNormal{};
		_float3 vStandPos{};
		_float  fHeight = 0.f;
	}GEO_TOP;

	typedef struct tagGeoLanding {
		_bool   hasSpace = false;
		_bool   isBlocked  = false;
		_bool   isElevated = false;
		_float3 vPos{};
	}GEO_LANDING;

	typedef struct tagObstacleGeometry {
		_float3     vTraversalDir{};
		GEO_FRONT   Front;
		GEO_TOP     Top;
		_bool       hasDepth = false;
		_float      fDepth = 0.f;
		_float      fTopWidth = 0.f;
		_bool       isPathBlocked  = false;
		_bool       isStandBlocked = false;
		GEO_LANDING Landing;
	}OBSTACLE_GEOMETRY;

	typedef struct tagEnvPerception {
		OBSTACLE_SCAN     Scan;
		OBSTACLE_GEOMETRY Geometry;
	}ENV_PERCEPTION;

	typedef struct tagWarpTarget
	{
		_float3 vPosition{};          
		_bool   hasRotation = false;
		_float4 qRotation{};          
		_bool   isValid = false;
	}WARP_TARGET;

	typedef struct tagClimbEval
	{
		_bool   isSupported  = false;
		_bool   isLanded     = false;
		_bool   shouldFall   = false;
		_bool   isArrived    = false;
		_bool   canMantle    = false;
		_bool   kneeHit      = false;
		_float3 vClimbNormal = {};
	}CLIMB_EVAL;

	typedef struct tagHangContext
	{
		_bool   isValid = false;
		_float3 vGrabEdgePos{};
		_float3 vWallNormal{};
		BodyID  GrabBodyID{};  // 지금 잡고 있는 장애물의 물리 바디
		void Reset() { isValid = false; GrabBodyID = BodyID{}; }
	}HANG_CONTEXT;
#pragma endregion

#pragma region STATE_ENTER
	struct STATE_ENTER_DESC
	{
		_uint iAnimIndex = UINT_MAX;
		_bool            hasEnvSnapshot = false;
		ENV_PERCEPTION   Perception{};
		PARKOUR_DECISION Decision{};
	};

	struct VAULT_ENTER_DESC : STATE_ENTER_DESC
	{
		_float3 vLandingPos;
		_float3 vObstacleTop;
	};
#pragma endregion

#pragma region IK
	typedef struct tagActiveIK
	{
		_string     strToken;        
		IK_TRIGGER  eTrigger;      
		_bool       isFixed;
		_bool		isResolved;
		_float3     vFixedPos;   
		_float3		vFixedNormal{ 0.f, 1.f, 0.f };
	}ACTIVE_IK;
#pragma endregion



}

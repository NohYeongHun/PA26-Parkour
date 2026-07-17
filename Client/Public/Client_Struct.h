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
	// 캐릭터 체형 프로필 — 초기화 시 콜라이더에서 1회 추출, 감지/결정이 읽기 전용 공유.
	// 값은 기존 EnvQuery 비율 상수(0.35/0.8/1.1/1.5 × 전고)와 동일해야 한다 (동작 보존).
	typedef struct tagBodyProfile
	{
		_float fHeight = 0.f;      // 전고 = Collider Height + 2 × Radius
		_float fRadius = 0.f;      // 캡슐 반지름
		_float fKneeHeight = 0.f;  // fHeight × 0.35
		_float fChestHeight = 0.f; // fHeight × 0.8
		_float fHeadHeight = 0.f;  // fHeight × 1.1
		_float fMaxReach = 0.f;    // fHeight × 1.5
	}BODY_PROFILE;

	typedef struct tagLineTraceHit
	{
		_bool		isHit = { false };
		_float      fCenterDistance = 0.f;
		_float3     vHitPosition{};
		_float3     vHitNormal{};
	}LINE_TRACE_HIT;

	enum class HEIGHT_HIT_FLAG : _uint { KNEE = 1 << 0, CHEST = 1 << 1, HEAD = 1 << 2 };

	// [EnvQuery 1단계 출력] 원시 스캔 — 측정 없음, 레이가 뭘 맞았는지만
	typedef struct tagObstacleScan {
		_bool          isObstacleDetected = false;              // ShapeCast 히트
		PARKOUR_FLAG   eObjectFlag = PARKOUR_FLAG::END;         // 디자이너 태그 (END = 태그 없음 → ALL 취급)
		LINE_TRACE_HIT KneeHit;
		LINE_TRACE_HIT ChestHit;
		LINE_TRACE_HIT HeadHit;
		LINE_TRACE_HIT LeftChestHit;
		LINE_TRACE_HIT RightChestHit;
		_uint          iHeightFlag = 0;                         // HEIGHT_HIT_FLAG 비트마스크
		_float3        vScanDir{};                              // EnvQuery가 사용한 스캔 방향 (Decider가 재계산 없이 소비)
	}OBSTACLE_SCAN;

	// [EnvQuery 3단계 출력] 액션별 판정 결과 — 탈락 시 첫 번째로 걸린 사유 기록
	typedef struct tagActionVerdict {
		_bool         isPossible = false;
		REJECT_REASON eReject = REJECT_REASON::NONE;
	}ACTION_VERDICT;

	typedef struct tagParkourDecision {
		ACTION_VERDICT Verdicts[ENUM_CLASS(PARKOUR_ACTION::END)];
		_uint          iCandidateFlag = 0;
		_float         fApproachDot = 0.f;
		PARKOUR_ACTION eBestEnvAction = PARKOUR_ACTION::NONE; // 舊 eBestAction — 환경상 최선 (입력 무관)
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
	}PARKOUR_DECISION;

	typedef struct tagObstacleGeometry {
		_bool   hasFront = false;          // ↓ 전면 그룹
		_float3 vFrontHitPos{};
		_float3 vFrontNormal{};
		_float3 vTraversalDir{};           // 장애물을 향하는 수평 진행 축 (전면 Normal 기반)
		_float  fFrontDistance = 0.f;

		_bool   isTopReachable = false;    // ↓ 상단 그룹
		_float3 vTopEdgePos{};             // 앞모서리 (모션워핑/커브 타겟)
		_float3 vTopNormal{};
		_float3 vTopStandPos;
		_float  fObstacleHeight = 0.f;     // 캐릭터 발 기준 상단면 높이

		_bool   hasDepth = false;          // ↓ 두께·폭 그룹
		_float  fDepth = 0.f;              // 전면에서 뒷모서리까지
		_float  fTopWidth = 0.f;           // 상단면 횡폭 (Task 6에서 측정)

		_bool   hasLandingSpace = false;   // ↓ 착지 그룹
		_float3 vLandingPos{};
	}OBSTACLE_GEOMETRY;

	// EnvQuery 출력 = 순수 관측 (Scan + Geometry). 판정은 CParkourDeciderComponent::PARKOUR_DECISION으로 분리됨.
	typedef struct tagEnvPerception {
		OBSTACLE_SCAN     Scan;
		OBSTACLE_GEOMETRY Geometry;
	}ENV_PERCEPTION;

	// 이름표 붙은 월드좌표 워프 타겟. CMotionWarpingComponent가 map<이름, WARP_TARGET>로 보유.
	typedef struct tagWarpTarget
	{
		_float3 vPosition{};          // 월드 목표 좌표
		_bool   hasRotation = false;  // 회전 워프 대상 여부 (이 슬라이스에선 미사용)
		_float4 qRotation{};          // 선택적 목표 방향
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


}

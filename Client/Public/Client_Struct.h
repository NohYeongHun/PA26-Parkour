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
	typedef struct tagLineTraceHit
	{
		_bool		isHit = { false };
		_float      fCenterDistance = 0.f;
		_float3     vHitPosition{};
		_float3     vHitNormal{};
	}LINE_TRACE_HIT;

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
		PARKOUR_ACTION eBestAction = PARKOUR_ACTION::NONE;
		_bool          isValid = false;
		_bool		   isTopReached = false;
	}PARKOUR_DECISION;

	// [EnvQuery 2단계 출력] 파생 기하 — 그룹별 유효성 플래그가 앞장섬 (false면 그룹 값 무효)
	typedef struct tagObstacleGeometry {
		_bool   hasFront = false;          // ↓ 전면 그룹
		_float3 vFrontHitPos{};
		_float3 vFrontNormal{};
		_float3 vTraversalDir{};           // 장애물을 향하는 수평 진행 축 (전면 Normal 기반)
		_float  fFrontDistance = 0.f;

		_bool   isTopReachable = false;    // ↓ 상단 그룹
		_float3 vTopEdgePos{};             // 앞모서리 (모션워핑/커브 타겟)
		_float3 vTopNormal{};
		_float  fObstacleHeight = 0.f;     // 캐릭터 발 기준 상단면 높이

		_bool   hasDepth = false;          // ↓ 두께·폭 그룹
		_float  fDepth = 0.f;              // 전면에서 뒷모서리까지
		_float  fTopWidth = 0.f;           // 상단면 횡폭 (Task 6에서 측정)

		_bool   hasLandingSpace = false;   // ↓ 착지 그룹
		_float3 vLandingPos{};
	}OBSTACLE_GEOMETRY;

	// EnvQuery 최종 결과 = 세 단계의 합성. 매 프레임 Execute 에 전체 리셋된다.
	typedef struct tagEnvQueryResult {
		OBSTACLE_SCAN     Scan;
		OBSTACLE_GEOMETRY Geometry;
		PARKOUR_DECISION  Decision;
	}ENV_QUERY_RESULT;

	typedef struct tagVaultPlan
	{
		_float3 vFaceDir;        
		_float3 vLandingPos;     
		_float3 vTopEdgePos;     
		_float  fRootMotionRate; 
	}VAULT_PLAN;

	typedef struct tagVaultTarget
	{
		_float3 vStartPos;    // 시작 위치
		_float3 vEndPos;	  // 착지 위치
		_float3 vObstacleTop; // 장애물 상단
	}VAULT_TARGET;

	typedef struct tagVaultMotion
	{
		_float3 Start;
		_float3 Apex;
		_float3 End;

		_float Duration;
		_float Elapsed;
	}VAULT_MOTION;


	typedef struct tagVaultCurve
	{
		_float3 P0;     // Start
		_float3 P1;     // Control
		_float3 P2;     // End
	}VAULT_CURVE;
#pragma endregion

#pragma region STATE_ENTER
	struct STATE_ENTER_DESC
	{
		_uint iAnimIndex = UINT_MAX;
	};

	struct VAULT_ENTER_DESC : STATE_ENTER_DESC
	{
		_float3 vLandingPos;
		_float3 vObstacleTop;
	};
#pragma endregion


}

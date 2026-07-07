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
	typedef struct tagObstacleGeometry {
		_float3 vTopEdgePos;      // 상단 모서리(모션워핑 타겟)
		_float3 vTopNormal;       // 상단면 노멀
		_float  fObstacleHeight;  // 캐릭터 발 기준 상단면 높이
		_float  fDepth;           // 장애물 두께
		_bool   hasLandingSpace;  // 반대편 착지 공간 유무
		_bool   isTopReachable;   // 상단 모서리가 최대 도달 높이 내에 있는지 (false면 벽타기 CLIMB만 가능)
	}OBSTACLE_GEOMETRY;

	typedef struct tagEnvQueryResult {
		_bool             isValid;       
		_uint             iCandidateFlag;
		PARKOUR_ACTION    eBestAction;     
		OBSTACLE_GEOMETRY Geometry;
	}ENV_QUERY_RESULT;


#pragma endregion


}

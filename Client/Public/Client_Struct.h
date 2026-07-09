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
		_float3 vTopEdgePos;      
		_float3 vTopNormal;
		_float3 vLandingPos;
		_float3 vFrontHitPos;
		_float3 vFrontNormal;
		_float3 vTraversalDir;
		_float  fObstacleHeight;  
		_float  fDepth;
		_float  fFrontDistance;
		
		_bool   hasLandingSpace;  
		_bool   isTopReachable;   
		
	}OBSTACLE_GEOMETRY;

	typedef struct tagEnvQueryResult {
		OBSTACLE_GEOMETRY Geometry;
		PARKOUR_ACTION    eBestAction = { PARKOUR_ACTION::NONE };
		_float			  fApproachDot;
		_uint             iCandidateFlag;
		_bool             isValid = { false };
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


}

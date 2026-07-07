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
		PARKOUR_FLAG eParkourFlag = { PARKOUR_FLAG::END };
	}CALLBACK_CLIENT;
}

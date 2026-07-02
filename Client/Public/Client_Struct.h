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
		_float fAttack = { 0.f };			 // 공격력
		_uint* pCondition = {};			// 컨디션 Value
		_wstring strEffectTag = {};		// 호출할 이펙트 태그
		OBJECTTYPE eObjectType{ OBJECTTYPE::END }; // 어떤 오브젝트인지 넣어서 판단하게. ANCHOR(고정), PULL(당긴다)
		// Shaking이나, HitStop? 이런 거.
		const _float4x4* pSocketMatrix = { nullptr }; // Grap 시 플레이어가 붙을 Matrix Pointer?

		SLIDE_DATA eSlideData;
		_bool		IsStart = { false };
		_bool* IsGrab = { nullptr };
		_bool* IsThrow = { nullptr };
		const _float4x4** ppRefBoneMatrix = { nullptr };
		const _float4x4** ppRefWorldMatrix = { nullptr };

		_wstring strSoundTag = {};
	}CALLBACK_CLIENT;
}

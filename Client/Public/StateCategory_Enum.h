#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)

#pragma region DEPTH 0
// 모든 캐릭터가 공유하는 최상위 상태 카테고리 (1 ~ 8 bit)
enum class EStateCategory : _uint
{
	GROUND = 1 << 0,	// 지상 상태
	AIR = 1 << 1,		// 공중 상태
	CLIMB = 1 << 2,		// 등반 상태
	CATEGORY_END
};
#pragma endregion



NS_END

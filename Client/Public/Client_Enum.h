#pragma once

namespace Client
{
	enum class LEVEL { STATIC, LOGO, GAMEPLAY, HEAVEN, LOADING, TEST, TEST_UI, END };
	enum class COLLISIONLAYER { NONE, MAP, PLAYER, PARKOUR, GRAB, DETECT, SLIDE, BURN,  END };
	//enum class OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, DESTRUCTION, NONRIGID, TRIGGERBOX, NONSONORA, NONSONORA_FLOOR, METEO, WATER, COLLAPS, THROW, BURN, DOME, TURN, ROPE_ANCHOR, ROPE_PULL, ROPE_UI, END };enum class OBJECTTYPE { DEFAULT, SONORA, INTERACTION, SPAWNOR, DESTRUCTION, NONRIGID, TRIGGERBOX, NONSONORA, NONSONORA_FLOOR, METEO, WATER, COLLAPS, THROW, BURN, DOME, TURN, ROPE_ANCHOR, ROPE_PULL, ROPE_UI, END };
	enum class OBJECTTYPE { DEFAULT, PARKOUR, GRAPPLE, END };
	enum class PARKOUR_FLAG {
		VAULTABLE	= 1 << 0, // 1
		CLIMBABLE	= 1 << 1, // 2
		HANGABLE	= 1 << 2, // 4
		HIGH_MANTLEABLE	= 1 << 3, // 8
		WALLRUNNABLE = 1 << 4, // 16
		LOW_MANTLEABLE = 1 << 5, // 32
		ALL = 0x3F, // 63
		END
	};

	// 이번 프레임에 실행 가능한 실제 파쿠르 액션.
	enum class PARKOUR_ACTION {
		NONE, LOW_VAULT, HIGH_VAULT, LOW_MANTLE, HIGH_MANTLE, CLIMB, HANG, WALL_RUN, END
	};

	// 액션 판정 탈락 사유
	enum class REJECT_REASON {
		NONE,             // 통과
		NO_HEIGHT_MATCH,  // 높이 패턴이 후보 조건에 안 맞음
		FLAG_DENIED,      // 디자이너 PARKOUR_FLAG가 금지
		TOP_UNREACHABLE,  // 상단면 도달 불가 (모서리 기반 액션 불가)
		NO_LANDING,       // 반대편 착지 공간 없음 (Vault)
		PATH_BLOCKED,     // 캐릭터 -> 착지 통로 막힘 (Vault)
		STAND_BLOCKED,    // 캐릭터 -> 설 자리 막힘 (Mantle — 올라설 헤드룸 없음)
		LANDING_ELEVATED, // 착지면이 장애물 상단 (Vault → LOW_MANTLE 전환 사유)
		TOO_THIN,         // 상단 두께 부족 (HIGH_MANTLE)
		TOO_NARROW,       // 상단 폭 부족 (HIGH_MANTLE)
		TOO_HIGH,         // 도달 불가 높이 (Climb)
		BAD_ANGLE,        // 접근 각도 미달
		NOT_ENOUGH_DISTANCE, // 충분한 거리 부족
		NOT_VERTICAL,     // 벽면이 수직이 아님 (WallRun)
		TOO_FAR,          // 벽까지 거리 초과 — 붙기 전 (WallRun)
		NOT_IMPLEMENTED,  // 미구현 액션 (HANG)
		END
	};

	enum class MOVEMENT_TYPE {
		GROUND,
		CLIMB,
		CLIMB_RUN,
		END
	};


	enum class INSTANCETYPE { DEFAULT, SONORO, NONSONORO, END };

	enum class SHADER_VTXANIMMESH_PATH { DEFAULT, NORMAL, END};

	enum class ACTORDIR { U, RU, R, RD, D, LD, L, LU, END };

	enum class SONORA { NONE, SONORA, END };

	/* 
	* VAULT : 넘을 수 있는지
	* CLIMBABLE : 벽을 탈 수 있는지 (높아서?)
	* HIGH_MANTLEABLE : 벽을 매달릴 수 있는지?
	*/
}
#pragma once

namespace Client
{
	enum class ROPEDIR : unsigned int {
		U = 0,
		F,
		D,
		END
	};

	enum class PROP_CONDITION : unsigned int {
		DEFAULT = 1 << 0,  
		DISSOLVE = 1 << 1, // Dissolve 설정.
		RESERVE_UNACTIVE =  1 << 2, // UnActive 예약.
		END
	};

	enum class CHARACTER_INTERACTIONTYPE : unsigned int {
		GRAPPLE = 0,		// 그래플
		SENSOR,			// 스캔
		FLIGHT,			// 활공
		LEVITATOR,		// 컨트롤
		NOTHING			// UI OFF 시 아무것도 선택되지 않음
	};

	enum class CHARACTER_TRANSITIONTYPE { IDLE, RUN, ATTACK, QTE, LEVIATAN_QTE, LEVIATAN_QTESUCCESS, LEVIATAN_PREV_EXECUTE, LEVIATAN_EXECUTE_SUCCESS, END }; // Character 전환시

	enum class PLAYER_CONDITION : unsigned int {
		LEVIATAN_GRAB = 0,
		LEVIATANQTE_START,
		LEVIATANQTE_SUCCESS,
		LEVIATANPREV_EXECUTE,
		LEVIATANEXECUTE_SUCCESS,
		GRABRELEASE,
		GRAB_UNBINED,
		TELEPORT,
		END
	};

	enum class CHARACTER_CONDITION : unsigned int {
		HIT = 1 << 0,
		DODGE = 1 << 1, // Dodge 상태면 Hit 안되게.
		DODGEABLE = 1 << 2, // Dodge 가능 상태.
		PARRY = 1 << 3,
		INVINCIBLE = 1 << 4,
		CHANGE = 1 << 5,
		CUTSCENE = 1 << 6,
		SKILLHIT = 1 << 7, // Skill에 맞았단 판정이 필요한 경우.(Galbrena Default E)
		SELECT = 1 << 8, // 선택된 상태.
		ROPE_HOOK = 1 << 9, // Rope 이동이 가능한상태?
		ROPE_DRAG = 1 << 10, // Rope Drag 상태.
		GRABED = 1 << 11, // Player가 Grab 된 상태
		STATE_DELAY = 1 << 12,  // Player의 State 속도가 저하된 상태.
		GRABRELEASE = 1 << 13,  // Player의 Grab이 해제된 상태.
		DISSOLVE = 1 << 14,
		LANDSLIDE_READY = 1 << 15, // Player에서 LandSlide 이벤트.
		LANDSLIDE = 1 << 16, // Player에서 LandSlide 이벤트.
		FPS = 1 << 17, // 1인칭 시점 상태.
		COLLIDER_UNACTIVE = 1 << 18, // 콜라이더 끈상태.8
		ANIMSTOP = 1<< 19, // Animation Stop 상태.
		CONTORL = 1 << 20,
		END
	};

	enum class UI_CHARACTERTYPE : unsigned int {
		ROVER = 0,
		AUGUSTA,
		GALBRENA,
		END
	};

	enum class KEYINPUT : unsigned int {
		NONE = 1 << 0,
		W = 1 << 1,
		S = 1 << 2,
		A = 1 << 3,
		D = 1 << 4,
		LB = 1 << 5,
		RB = 1 << 6,
		WB = 1 << 7,
		Q = 1 << 8,
		E = 1 << 9,
		R = 1 << 10,
		T = 1 << 11,
		LSHIFT = 1 << 12,
		SPACE = 1 << 13,
		D1 = 1 << 14,
		D2 = 1 << 15,
		D3 = 1 << 16,
		D4 = 1 << 17,
		D5 = 1 << 18,
		D6 = 1 << 19,
		G = 1 << 20,
		F6 = 1 << 21,
	};

	typedef enum class ESkillType : _uint
	{
		NONE = 0,	    // (비용 없음)
		RESONANCE,		// 공명 게이지
		AUGUSTA_POINT,	// 특수 원형 게이지
		AUGUSTA_ULTI,	// 기본 궁극기 게이지
		AUGUSTA_SWORD,	// 특수 칼 게이지 (갯수)
		AUGUSTA_UITI_SWORD, // 칼 사용 후 썼을때 공격 게이지?
		SKILL_TYPE_END // CSV 파싱 실패 등을 위한 END
	}SKILL_TYPE;

	typedef enum class ECostType : unsigned int
	{
		NONE = 0, // Stat 아님.
		COST1, // 1 RESONANCE
		COST2, // 2 AUGUSTA POINT
		COST3, // 3 AUGUSTA_SWORD
		COST4, // 4 AUGUSTA_ULTI_SWORD
		COST5, // 5. AUGUTA_ULTI
		STAMINA, // 5
		COST_TYPE_END
	}COST_TYPE;

	enum class SKILL_STATE : unsigned int
	{
		READY = 0,		 // 사용 가능
		COOLING_DOWN,	 // 쿨타임
		NOT_ENOUGH_COST, // 자원 부족
		NOT_EXIST,		 // 정보가 없음.
		END
	};

	
}
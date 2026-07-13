#pragma once
#include "Client_Define.h"
#include "StateCategory_Enum.h"


NS_BEGIN(Client)

#pragma region DEPTH 1
enum class ETraceurGroundState : _uint
{

	Move = 0,	// 대기/걷기/달리기 통합
	Stand,			// 전환용.
	Sprint,			// 전력질주
	Land,			// 착지
	Vault,			// 장애물 넘기
	GroundEnd
};


enum class ETraceurAirState : _uint
{
	Jump = 0, // 점프
	Fall,// 추락
	AirEnd
};

enum class ETraceurClimbState : _uint
{
	Enter = 0, // 등반 매달리기 => 시작
	Move,	  // 등반 이동 => 5개 모션 섞을 예정 (L, U, R, D, Idle)
	Mantle,   // 탈출 상태.
	Exit,     // 벽에서 이탈
	ClimbEnd
};
#pragma endregion


#pragma region DEPTH 2

#pragma region GROUND
enum class ETraceurGroundMove : _uint
{
	Move = 0,
	MoveEnd
};

enum class ETraceurGroundVault : _uint
{
	LowerVault = 0, // 애니메이션 명
	VaultEnd
};

enum class ETraceurGroundLand : _uint
{
	FallingToLanding = 0,
	FallALandToStandingIdle,
	MoveEnd
};

enum class ETraceurGroundStand : _uint
{
	ClimbingStand = 0,
	StandingIdleToActionIdle,
	MoveEnd
};

#pragma endregion

#pragma region AIR
enum class ETraceurAirJump : _uint
{
	Jump = 0,
	BackFlip,
	Jump_End
};

enum class ETraceurAirFall : _uint
{
	FallingIdle = 0,
	FallALoop,
	JumpFromWall,	
	Fall_End
};
#pragma endregion



#pragma region CLIMB
enum class ETraceurClimbEnter : _uint
{
	IdleToBracedHang = 0,
	EnterEnd
};

enum class ETraceurClimbMove : _uint // 2D Blend Space 예정
{
	HangingIdle = 0,
	Move,
	MoveEnd
};

enum class ETraceurClimbMantle : _uint // 암벽 등반을 끝까지 올라갔을 때.
{
	Mantle = 0,
	Mantle_End
};

enum class ETraceurClimbExit : _uint
{
	JumpFromWall = 0,
	Climbing, // Mantle
	ClimbExitEnd
};

#pragma endregion




#pragma endregion


NS_END
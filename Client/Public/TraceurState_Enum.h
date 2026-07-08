#pragma once
#include "Client_Define.h"
#include "StateCategory_Enum.h"


NS_BEGIN(Client)

#pragma region DEPTH 1
enum class ETraceurGroundState : _uint
{
	Idle = 0,	// 대기 
	Walk,	// 걷기
	Run,	// 달리기
	Sprint,// 전력질주 (Sprint_F, Super_Sprint_*)
	Land,	// 착지 (Land)
	Vault,// 장애물 넘기
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
	Idle =0,	// 등반 대기
	Move,		// 등반 이동
	Hange,		// 등반 매달리기
	Exit,		// 등반 탈출
	ClimbEnd
};
#pragma endregion


#pragma region DEPTH 2
enum class ETraceurGroundIdle : _uint
{
	Idle = 0, // 애니메이션 명
	idleEnd
};

enum class ETraceurGroundRun : _uint
{
	Run = 0, // 애니메이션 명
	RunEnd
};

enum class ETraceurGroundWalk : _uint
{
	Walking = 0, // 애니메이션 명
	WalkEnd
};

enum class ETraceurGroundVault: _uint
{
	LowerVault = 0, // 애니메이션 명
	VaultEnd
};


#pragma endregion


NS_END
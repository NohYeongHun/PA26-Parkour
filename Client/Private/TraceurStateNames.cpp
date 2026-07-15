#include "ClientPch.h"
#include "TraceurStateNames.h"
#include "TraceurState_Enum.h"

using Engine::StateKey;

namespace
{
	const map<_string, _uint>& Category_Table()
	{
		static const map<_string, _uint> Table = {
			{ "GROUND", ENUM_CLASS(EStateCategory::GROUND) },
			{ "AIR",    ENUM_CLASS(EStateCategory::AIR) },
			{ "CLIMB",  ENUM_CLASS(EStateCategory::CLIMB) },
		};
		return Table;
	}

	const map<_uint, map<_string, _uint>>& SubState_Table()
	{
		static const map<_uint, map<_string, _uint>> Table = {
			{ ENUM_CLASS(EStateCategory::GROUND), {
				{ "Move",   ENUM_CLASS(ETraceurGroundState::Move) },
				{ "Stand",   ENUM_CLASS(ETraceurGroundState::Stand) },
				{ "Sprint", ENUM_CLASS(ETraceurGroundState::Sprint) },
				{ "Land",   ENUM_CLASS(ETraceurGroundState::Land) },
				{ "Vault",  ENUM_CLASS(ETraceurGroundState::Vault) },
			}},
			{ ENUM_CLASS(EStateCategory::AIR), {
				{ "Jump", ENUM_CLASS(ETraceurAirState::Jump) },
				{ "Fall", ENUM_CLASS(ETraceurAirState::Fall) },
			}},
			{ ENUM_CLASS(EStateCategory::CLIMB), {
				{ "Enter",  ENUM_CLASS(ETraceurClimbState::Enter) },
				{ "Move",   ENUM_CLASS(ETraceurClimbState::Move) },
				{ "Mantle", ENUM_CLASS(ETraceurClimbState::Mantle) },
				{ "Exit",   ENUM_CLASS(ETraceurClimbState::Exit) },
				{ "Run",    ENUM_CLASS(ETraceurClimbState::Run) },
			}},
		};
		return Table;
	}

	const map<StateKey, map<_string, _uint>>& Anim_Table()
	{
		static const map<StateKey, map<_string, _uint>> Table = {
			{ { ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move) }, {
				{ "Move", ENUM_CLASS(ETraceurGroundMove::Move) },
			}},
			{ { ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Stand) }, {
				{ "ClimbingStand", ENUM_CLASS(ETraceurGroundStand::ClimbingStand) },
				{ "StandingIdleToActionIdle", ENUM_CLASS(ETraceurGroundStand::StandingIdleToActionIdle) },
			}},

			{ { ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Vault) }, {
				{ "LowerVault", ENUM_CLASS(ETraceurGroundVault::LowerVault) },
			}},
			{ { ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Land) }, {
				{ "Landing",        ENUM_CLASS(ETraceurGroundLand::Landing) },
				{ "FallingToLanding",        ENUM_CLASS(ETraceurGroundLand::FallingToLanding) },
				{ "FallALandToStandingIdle", ENUM_CLASS(ETraceurGroundLand::FallALandToStandingIdle) },
			}},
			{ { ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Jump) }, {
				{ "Jump",     ENUM_CLASS(ETraceurAirJump::Jump) },
				{ "BackFlip", ENUM_CLASS(ETraceurAirJump::BackFlip) },
			}},
			{ { ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall) }, {
				{ "FallingIdle",  ENUM_CLASS(ETraceurAirFall::FallingIdle) },
				{ "FallALoop",    ENUM_CLASS(ETraceurAirFall::FallALoop) },
				{ "JumpFromWall", ENUM_CLASS(ETraceurAirFall::JumpFromWall) },
			}},
			{ { ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Enter) }, {
				{ "IdleToBracedHang", ENUM_CLASS(ETraceurClimbEnter::IdleToBracedHang) },
			}},
			{ { ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Move) }, {
				{ "HangingIdle", ENUM_CLASS(ETraceurClimbMove::HangingIdle) },
				{ "Move",        ENUM_CLASS(ETraceurClimbMove::Move) },
			}},
			{ { ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Mantle) }, {
				{ "Mantle", ENUM_CLASS(ETraceurClimbMantle::Mantle) },
			}},
			{ { ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Exit) }, {
				{ "Climbing", ENUM_CLASS(ETraceurClimbExit::Climbing) },
				{ "BracedHangDrop", ENUM_CLASS(ETraceurClimbExit::BracedHangDrop) },
				{ "BracedHangToCrouch", ENUM_CLASS(ETraceurClimbExit::BracedHangToCrouch) },
				{ "ClimbingToTop", ENUM_CLASS(ETraceurClimbExit::ClimbingToTop) }
			}},
			{ { ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Run) }, {
				{ "Move", ENUM_CLASS(ETraceurClimbRun::Move) },
				{ "WallRunUp", ENUM_CLASS(ETraceurClimbRun::WallRunUp) },
			}},
		};
		return Table;
	}
}

_bool CTraceurStateNames::Resolve_StateKey(const _string& strPath, StateKey& OutKey)
{
	const size_t iSlash = strPath.find('/');
	if (iSlash == _string::npos)
		return false;

	const _string strCategory = strPath.substr(0, iSlash);
	const _string strSubState = strPath.substr(iSlash + 1);

	const auto& Categories = Category_Table();
	const auto itCategory = Categories.find(strCategory);
	if (itCategory == Categories.end())
		return false;

	const auto itSubStates = SubState_Table().find(itCategory->second);
	if (itSubStates == SubState_Table().end())
		return false;

	const auto itSubState = itSubStates->second.find(strSubState);
	if (itSubState == itSubStates->second.end())
		return false;

	OutKey = StateKey(itCategory->second, itSubState->second);
	return true;
}

_bool CTraceurStateNames::Resolve_AnimIndex(const StateKey& Key, const _string& strAnim, _uint& iOutIndex)
{
	const auto& Anims = Anim_Table();
	const auto itState = Anims.find(Key);
	if (itState == Anims.end())
		return false;

	const auto itAnim = itState->second.find(strAnim);
	if (itAnim == itState->second.end())
		return false;

	iOutIndex = itAnim->second;
	return true;
}

_string CTraceurStateNames::To_String(const StateKey& Key)
{
	_string strCategory = "?";
	for (const auto& [strName, iValue] : Category_Table())
		if (iValue == Key.iCategory) { strCategory = strName; break; }

	_string strSubState = "?";
	const auto& SubStates = SubState_Table();
	const auto itCategory = SubStates.find(Key.iCategory);
	if (itCategory != SubStates.end())
		for (const auto& [strName, iValue] : itCategory->second)
			if (iValue == Key.iSubState) { strSubState = strName; break; }

	return strCategory + "/" + strSubState;
}

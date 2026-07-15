#include "ClientPch.h"
#include "TraceurFactory.h"
#include "Traceur.h"
#include "InputController.h"
#include "SpringCamera.h"

#include "TraceurState_Enum.h"
#include "TraceurState.h"

// Ground State
#include "TraceurGroundMove.h"
#include "TraceurGroundVault.h"
#include "TraceurGroundLand.h"
#include "TraceurGroundStand.h"

// Climb State
#include "TraceurClimbEnter.h"
#include "TraceurClimbMove.h"
#include "TraceurClimbExit.h"
#include "TraceurClimbRun.h"

// Air State
#include "TraceurAirFall.h"
#include "TraceurAirJump.h"


void CTraceurFactory::Register_KeyInputs(CInputController* pInputControllerCom, CTraceur* pTraecur)
{
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::W), DIK_W);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::A), DIK_A);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::S), DIK_S);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D), DIK_D);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::SPACE), DIK_SPACE);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::LSHIFT), DIK_LSHIFT);

	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D1), DIK_1);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D2), DIK_2);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D3), DIK_3);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D4), DIK_4);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D5), DIK_5);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::D6), DIK_6);

	//pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::G), DIK_G); // 임시 추가.
	//pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::F6), DIK_F6);
	pInputControllerCom->Register_KeyBoardKeyInput(ENUM_CLASS(KEYINPUT::F9), DIK_F9);


	pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::LB), MOUSEKEYSTATE::LB);
	pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::WB), MOUSEKEYSTATE::WB);
	pInputControllerCom->Register_MouseKeyInput(ENUM_CLASS(KEYINPUT::RB), MOUSEKEYSTATE::RB);
}

void CTraceurFactory::Register_Camera(LEVEL ePrototypeLevel, LEVEL eLevel, CTraceur* pTraecur, CGameInstance* pGameInstance, CSpringCamera** ppCamera)
{
	CSpringCamera::CAMERA_DESC CameraDesc{};
	CameraDesc.fSpeedPerSec = 100.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 2000.f;
	CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
	CameraDesc.fMouseSensor = 0.002f;

	CSpringCamera* pSpringCamera = dynamic_cast<CSpringCamera*>(pGameInstance->Clone_Prototype(ENUM_CLASS(ePrototypeLevel)
		, TEXT("Prototype_GameObject_SpringCamera"), PROTOTYPE::GAMEOBJECT
		, &CameraDesc));

	ASSERT_CRASH(pSpringCamera);
	*ppCamera = pSpringCamera;

	pGameInstance->Add_Camera(ENUM_CLASS(eLevel), TEXT("Camera_Spring"), pSpringCamera);
	Safe_AddRef(pSpringCamera);

	pGameInstance->Change_MainCamera(ENUM_CLASS(eLevel), TEXT("Camera_Spring"));
}

void CTraceurFactory::Register_States(CStateMachine* pStateMachineCom, CTraceur* pCharacter)
{
	auto AddState = [&](_uint iCategory, _uint iSubState, CTraceurState* pState)
	{
		pState->Set_SelfKey({ iCategory, iSubState });
		pStateMachineCom->Add_State(iCategory, iSubState, pState);
	};

	AddState(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move),  CTraceurGroundMove::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Stand), CTraceurGroundStand::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Vault), CTraceurGroundVault::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Land),  CTraceurGroundLand::Create(pCharacter));
	

	AddState(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Enter), CTraceurClimbEnter::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Move),  CTraceurClimbMove::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Exit),  CTraceurClimbExit::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::CLIMB), ENUM_CLASS(ETraceurClimbState::Run),   CTraceurClimbRun::Create(pCharacter));

	AddState(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall), CTraceurAirFall::Create(pCharacter));
	AddState(ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Jump), CTraceurAirJump::Create(pCharacter));

	pStateMachineCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
}

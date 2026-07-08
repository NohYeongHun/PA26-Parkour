#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CTraceurFactory : public CBase
{
public:
	static void Register_KeyInputs(class CInputController* pInputControllerCom, class CTraceur* pTraecur);
	static void Register_Camera(LEVEL ePrototypeLevel, LEVEL eLevel, class CTraceur* pTraceur, class CGameInstance* pGameInstance, class CSpringCamera** ppCamera);
	static void Register_States(class CStateMachine* pStateMachineCom, class CTraceur* pCharacter);
};
NS_END


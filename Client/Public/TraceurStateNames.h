#pragma once
#include "Client_Define.h"
#include "StateMachine.h"

NS_BEGIN(Client)

class CTraceurStateNames final
{
public:
	static _bool Resolve_StateKey(const _string& strPath, Engine::StateKey& OutKey);
	static _bool Resolve_Category(const _string& strName, _uint& iOutCategory);
	static _bool Resolve_AnimIndex(const Engine::StateKey& Key, const _string& strAnim, _uint& iOutIndex);
	static _string To_String(const Engine::StateKey& Key);
};

NS_END

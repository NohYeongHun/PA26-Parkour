#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class CAnimState;
class CAnimTransition;

class CAnimStateFactory final : public CBase
{
public:
	enum CONDITION_TYPE {SINGLE, MULTI, NOT};

public:
	static function<_bool(const _uint*, const _uint)> Register_Transition(json& jsonTransition);
};

NS_END
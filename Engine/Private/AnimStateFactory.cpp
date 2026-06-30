#include "EnginePch.h"
#include "AnimStateFactory.h"

function<_bool(const _uint*, const _uint)> CAnimStateFactory::Register_Transition(json& jsonTransition)
{
	_uint eType = jsonTransition["Condition Type"];
	switch(CONDITION_TYPE(eType))
	{
	case SINGLE:
	{
		auto Func = +[](const _uint* pState, const _uint iTarget) ->_bool{
			return (*pState & iTarget);
			};
		return Func;
	}
	case MULTI:
	{
		auto Func = +[](const _uint* pState, const _uint iTarget) ->_bool{
			return (*pState == iTarget);
			};
		return Func;
	}
	case NOT:
	{
		auto Func = +[](const _uint* pState, const _uint iTarget) ->_bool{
			return !(*pState & iTarget);
			};
		return Func;
	}
	default:
		return nullptr;
	}
}

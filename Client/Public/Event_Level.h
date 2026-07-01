#pragma once
#include "ClientPch.h"

typedef struct tagChangeLevel : public CEvent
{
	LEVEL eNextLevel;
	_bool isLoad;
	tagChangeLevel(LEVEL _eNextLevel, _bool _isLoad)
		: eNextLevel{ _eNextLevel }, isLoad{ _isLoad } {};
}CHANGE_LEVEL_EVENT;

typedef struct tagLoadingEnd : public CEvent
{
	_bool isFinish;
	tagLoadingEnd(_bool _isFinish) : isFinish{ _isFinish } {};
}LOADING_END_EVENT;

typedef struct tagOnClickEnterUI : public CEvent
{	// m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), L"Event_OnClickEnterUI", ONCLICKENTER_UI_EVENT(iInstanceIndex));
	_uint iInstanceIndex;
	tagOnClickEnterUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKENTER_UI_EVENT;
typedef struct tagOnClickingUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnClickingUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKING_UI_EVENT;
typedef struct tagOnClickExitUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnClickExitUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONCLICKEXIT_UI_EVENT;

typedef struct tagOnHoverEnterUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoverEnterUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVERENTER_UI_EVENT;
typedef struct tagOnHoveringUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoveringUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVERING_UI_EVENT;
typedef struct tagOnHoverExitUI : public CEvent
{
	_uint iInstanceIndex;
	tagOnHoverExitUI(_uint iInstanceIndex = 0) : iInstanceIndex{ iInstanceIndex } {};
}ONHOVEREXIT_UI_EVENT;


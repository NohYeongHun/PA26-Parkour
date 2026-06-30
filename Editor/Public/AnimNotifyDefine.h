#pragma once
#include "EnginePch.h"
// ?대뼡 ?쒓렇瑜??쒖꽦???섎뒗吏????댁꽌留?吏??

typedef struct tagBaseNotifiy
{
	_float  fTrackPosition = 0.f;
}BASENOTIFY;

typedef struct tagSoundNotify : BASENOTIFY
{
	_string strSoundTag;
	_string strSoundType;
	_float fVolume = 0.1f;
}SOUNDNOTIFY;

typedef struct tagEffectNotify : BASENOTIFY
{
	// 1. Effect Type
	_string strEffectTag;

	// 2. Bone ?ъ슜 ?щ?
	_bool IsUseBone = { false };
	_string strBoneName = "";

	// 3. ?먯꽭???ㅼ젙媛믪? ?鍮꾧? ?ㅼ젙?⑹떆??. => ?대옒???ㅺ퀎 ?곕씪 ?덈Т ?щ씪吏?
	
}EFFECTNOTIFY;

typedef struct tagColliderNotify : BASENOTIFY
{
	// 1. ?꾧퀬 耳쒓퀬 湲곕뒫留??덉쑝硫??좊벏.
	_string strColliderTag;
	_bool IsActive = { false }; 
}COLLIDERNOTIFY;

typedef struct tagLightNotify : BASENOTIFY
{
	// 1. ?꾧퀬 耳쒓퀬 湲곕뒫留??덉쑝硫??좊벏.
	_string strLightTag;
	_bool IsActive = { false };
}LIGHTNOTIFY;

typedef struct tagObjectNotify : BASENOTIFY
{
	_string strColliderTag;
}OBJECTNOTIFY;
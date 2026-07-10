#include "EnginePch.h"
#include "State.h"
#include "StateMachine.h"


HRESULT CState::Initialize(CGameObject* pOwner)
{
    return S_OK;
}

void CState::OnEnter(void* pArg)
{
    m_fTrackPosition = 0.f;
    m_IsAnimationEnd = false;
}


void CState::OnUpdate(_float fTimeDelta)
{

}


void CState::OnExit()
{

}

void CState::Change_State(CStateMachine* pStateMachine, _uint iCategory, _uint iSubState)
{
    pStateMachine->Change_State(iCategory, iSubState);
}



_bool CState::Is_EscapePossible()
{
    return m_fTrackPosition > m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.fEscapeTrackPosition;
}

void CState::Add_Animations(_uint iType, const ANIMATION_PLAY_DESC& AnimPlayDesc, const ROOTMOTION_DESC& RootMotionDesc)
{
	ANIM_DATA Data{};
	Data.eType = EAnimSlotType::CLIP;
	Data.AnimPlayDesc = AnimPlayDesc;
	Data.RootMotionDesc = RootMotionDesc;
	m_Animations.emplace(iType, Data);
}

void CState::Add_ParkourAnimations(_uint iType, const ANIMATION_PLAY_DESC& AnimPlayDesc, const ROOTMOTION_DESC& RootMotionDesc, const PARKOUR_ANIM_DESC& ParkourAnimDesc)
{
	ANIM_DATA Data{};
	Data.eType = EAnimSlotType::CLIP;
	Data.AnimPlayDesc = AnimPlayDesc;
	Data.RootMotionDesc = RootMotionDesc;
	Data.ParkourAnimDesc = ParkourAnimDesc;
	m_Animations.emplace(iType, Data);
}

void CState::Add_BlendSpace(_uint iType, const BLENDSPACE_1D_DESC& BlendSpaceDesc, const ROOTMOTION_DESC& RootMotionDesc)
{
	ANIM_DATA Data{};
	Data.eType = EAnimSlotType::BLENDSPACE_1D;
	Data.BlendSpaceDesc = BlendSpaceDesc;
	Data.RootMotionDesc = RootMotionDesc;
	m_Animations.emplace(iType, Data);
}

void CState::Add_BlendSpace(_uint iType, const BLENDSPACE_2D_DESC& BlendSpaceDesc, const ROOTMOTION_DESC& RootMotionDesc)
{
	ANIM_DATA Data{};
	Data.eType = EAnimSlotType::BLENDSPACE_2D;
	Data.BlendSpace2Desc = BlendSpaceDesc;
	Data.RootMotionDesc = RootMotionDesc;
	m_Animations.emplace(iType, Data);
}


//void CState::Add_Animations(_uint iType, const _string& strAnimName, _float fSpeed, _float fEscapeTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate)
//{
//    m_Animations.emplace(iType, ANIM_DATA{ strAnimName, fSpeed, fEscapeTrackPosition, fRootMotionRate, IsRootMotion, IsRootMotionRotate, IsRootMotionTranslate });
//}

void CState::Free()
{
    CBase::Free();
}

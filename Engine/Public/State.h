
#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CState abstract : public CBase
{
public:
	enum class EAnimSlotType { CLIP, BLENDSPACE_1D };

    typedef struct tagAnimData
    {
		EAnimSlotType eType = EAnimSlotType::CLIP;
		ANIMATION_PLAY_DESC AnimPlayDesc{};
		ROOTMOTION_DESC RootMotionDesc{};
		BLENDSPACE_1D_DESC BlendSpaceDesc{};
    }ANIM_DATA;


protected:
    explicit CState() = default;
    virtual ~CState() = default;

public:
    virtual HRESULT Initialize(class CGameObject* pOwner);
    virtual void OnEnter(void* pArg = nullptr);
    virtual void OnUpdate(_float fTimeDelta); // Update
    virtual void OnExit(); // Exit

    // HSM: 계층적 State 전환 (Category + SubState)
    virtual void Change_State(class CStateMachine* pStateMachine, _uint iCategory, _uint iSubState);
    
    


protected:
    _float m_fTrackPosition = {};
    _bool m_IsAnimationEnd = { false };
    _uint m_iCurrentAnimIdx = {};
	
    // 현재 재생 중인 애니메이션
    map<_uint, ANIM_DATA> m_Animations;  // 이 State가 사용하는 애니메이션들

protected:
    virtual void Check_State() {};
    _bool Is_EscapePossible();

	void Add_Animations(_uint iType, const ANIMATION_PLAY_DESC& AnimPlayDesc, const ROOTMOTION_DESC& RootMotionDesc);
	void Add_BlendSpace(_uint iType, const BLENDSPACE_1D_DESC& BlendSpaceDesc, const ROOTMOTION_DESC& RootMotionDesc);

public:
    virtual void Free() override;
};
NS_END


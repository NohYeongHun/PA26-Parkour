
#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CState abstract : public CBase
{
public:
	enum class EAnimSlotType { CLIP, BLENDSPACE_1D, BLENDSPACE_2D };

    typedef struct tagAnimData
    {
		EAnimSlotType eType = EAnimSlotType::CLIP;
		ANIMATION_PLAY_DESC AnimPlayDesc{};
		ROOTMOTION_DESC RootMotionDesc{};
		BLENDSPACE_1D_DESC BlendSpaceDesc{};
		BLENDSPACE_2D_DESC BlendSpace2Desc{};
		PARKOUR_ANIM_DESC ParkourAnimDesc{};
    }ANIM_DATA;


protected:
    explicit CState() = default;
    virtual ~CState() = default;

public:
	virtual _uint Get_CurrentAnimIndex() { return UINT_MAX; }

public:
    virtual HRESULT Initialize(class CGameObject* pOwner);
    virtual void OnEnter(void* pArg = nullptr);
    virtual void OnUpdate(_float fTimeDelta); // Update
    virtual void OnExit(); // Exit

    // HSM: 계층적 State 전환 (Category + SubState)
    virtual void Change_State(class CStateMachine* pStateMachine, _uint iCategory, _uint iSubState);

protected:
    virtual void Check_State() {};

public:
    virtual void Free() override;
};
NS_END


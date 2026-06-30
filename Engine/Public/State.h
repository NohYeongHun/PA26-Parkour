
#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CState abstract : public CBase
{
public:
    typedef struct tagAnimData
    {
        _string strAnimName = {};
        _float fSpeed = {};
        _float fEscapeTrackPosition = {};
        _float fRootMotionRate = { 1.f };
        _bool IsRootMotion = { true };
        _bool IsRootMotionRotate = { true };
        _bool IsRootMotionTranslate = { true };
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
    virtual void Handle_Input() {};
    _bool Is_EscapePossible();
    void Add_Animations(_uint iType, const _string& strAnimName, _float fSpeed, _float fEscapeTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true);

public:
    virtual void Free() override;
};
NS_END


#pragma once
#include "Base.h"

NS_BEGIN(Engine)
// 구간(윈도우) 노티 기반 클래스. 
// 어떤 종료 에서든 End를 보장
class ENGINE_DLL CAnimNotifyState abstract : public CBase
{
public:
    explicit CAnimNotifyState(_float fBeginTrackPos, _float fEndTrackPos);
    virtual ~CAnimNotifyState() = default;
    
    void Notify_Begin() { m_isActive = true;  On_NotifyBegin(); }
    void Notify_End()   { m_isActive = false; On_NotifyEnd(); }

    virtual json To_Json() const = 0;
    virtual const _string& Get_NotifyTypeName() const = 0;

#ifdef _DEBUG
    virtual void ImGui_Print() = 0;
#endif // _DEBUG

    _float Get_BeginTrackPos() const { return m_fBeginTrackPosition; }
    _float Get_EndTrackPos() const { return m_fEndTrackPosition; }
    _bool  Is_Active() const { return m_isActive; }

protected:
    virtual void On_NotifyBegin() = 0;
    virtual void On_NotifyEnd() = 0;

protected:
    _float  m_fBeginTrackPosition = {};
    _float  m_fEndTrackPosition = {};
    _bool   m_isActive = false;
    _string m_strNotifyTypeName = {};

public:
    virtual void Free() override;
};
NS_END

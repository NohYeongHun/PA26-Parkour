#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CAnimNotify abstract: public CBase
{
public:
    explicit CAnimNotify(_float fTrackPosition);
    virtual ~CAnimNotify() = default;
    
    virtual void Execute() = 0;
    virtual json To_Json() const = 0;         
    virtual const _string& Get_NotifyTypeName() const = 0;

    void Set_ColliderCallBack(function<void(const _wstring&, _bool)> ColliderCallback) { m_ColliderCallback = ColliderCallback;  }
    void Set_EffectCallback(function<void(const _wstring&)> EffectCallback) { m_EffectCallback = EffectCallback; }

#ifdef _DEBUG
    virtual void ImGui_Print() = 0;
#endif // _DEBUG

    const _float Get_TrackPosition() const { return m_fTrackPosition; }

protected:
    _float m_fTrackPosition = {};
    _string m_strNotifyTypeName = {};
    function<void(const _wstring&, _bool)> m_ColliderCallback;
    function<void(const _wstring&)> m_EffectCallback;



public:
    virtual void Free() override;
};
NS_END

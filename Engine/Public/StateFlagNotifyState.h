#pragma once
#include "AnimNotifyState.h"

NS_BEGIN(Engine)
// 구간 StateFlag: Begin = 플래그 on, End = 플래그 off.
class ENGINE_DLL CStateFlagNotifyState final : public CAnimNotifyState
{
public:
    explicit CStateFlagNotifyState(_float fBeginTrackPos, _float fEndTrackPos, const _string& strFlagName);
    json To_Json() const override;
    const _string& Get_NotifyTypeName() const override;

    void Set_StateFlagCallback(function<void(const _string&, _bool)> Callback) { m_StateFlagCallback = Callback; }

#ifdef _DEBUG
    void ImGui_Print() override;
#endif // _DEBUG

protected:
    void On_NotifyBegin() override;
    void On_NotifyEnd() override;

public:
    static CStateFlagNotifyState* From_Json(const json& j);
    virtual void Free() override;

private:
    function<void(const _string&, _bool)> m_StateFlagCallback;
    _string m_strFlagName = {};
};
NS_END

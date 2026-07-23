#pragma once
#include "AnimNotifyState.h"

NS_BEGIN(Engine)
// 구간 IK: Begin = goal 활성, End = goal 해제
class ENGINE_DLL CIKNotifyState final : public CAnimNotifyState
{
public:
    explicit CIKNotifyState(_float fBeginTrackPos, _float fEndTrackPos);
    json To_Json() const override;
    const _string& Get_NotifyTypeName() const override;

    void Set_IKCallback(function<void(const vector<IK_BINDING>& bindings, _float fBlendSec, _bool isBegin)> Callback) { m_IKCallBack = Callback; }

    const vector<IK_BINDING>& Get_Bindings() const { return m_Bindings; }
    _float Get_BlendInSec() const { return m_fBlendInSec; }
    _float Get_BlendOutSec() const { return m_fBlendOutSec; }
    _float Get_RampLen() const { return m_fRampLen; }

#ifdef _DEBUG
    void ImGui_Print() override;
#endif // _DEBUG

protected:
    void On_NotifyBegin() override;
    void On_NotifyEnd() override;

public:
    static EIKTARGET_MODE To_Mode(const string& s);
    static _string Mode_ToString(EIKTARGET_MODE eMode);
    static CIKNotifyState* From_Json(const json& j);
    virtual void Free() override;

private:
    function<void(const vector<IK_BINDING>& bindings, _float fBlendSec, _bool isBegin)> m_IKCallBack;
    vector<IK_BINDING> m_Bindings{};
    _float m_fBlendInSec = {};
    _float m_fBlendOutSec = {};
    _float m_fRampLen = {};		// 트랙 단위 알파 램프 길이 (0 = 구간 전체가 램프)
};
NS_END

#pragma once
#include "AnimNotifyState.h"

NS_BEGIN(Engine)
// 구간 MotionWarp: Begin = 워프 시작(isStart=true), End = 워프 종료(isStart=false).
class ENGINE_DLL CMotionWarpNotifyState final : public CAnimNotifyState
{
public:
    explicit CMotionWarpNotifyState(_float fBeginTrackPos, _float fEndTrackPos,
                                    const _string& strTargetName,
                                    _bool isWarpTranslation, _bool isWarpRotation);
    json To_Json() const override;
    const _string& Get_NotifyTypeName() const override;

    void Set_WarpCallback(function<void(const _string&, _bool, _float, _bool, _bool)> cb)
    { m_WarpCallback = cb; }

#ifdef _DEBUG
    void ImGui_Print() override;
#endif // _DEBUG

protected:
    void On_NotifyBegin() override;
    void On_NotifyEnd() override;

public:
    static CMotionWarpNotifyState* From_Json(const json& j);
    virtual void Free() override;

private:
    function<void(const _string& strTargetName, _bool isStart,
                  _float fWindowEndTrackPos, _bool bTrans, _bool bRot)> m_WarpCallback;
    _string m_strTargetName{};
    _bool   m_bWarpTranslation = true;
    _bool   m_bWarpRotation = false;
};
NS_END

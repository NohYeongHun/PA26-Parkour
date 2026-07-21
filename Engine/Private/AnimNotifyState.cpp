#include "EnginePch.h"
#include "AnimNotifyState.h"

CAnimNotifyState::CAnimNotifyState(_float fBeginTrackPos, _float fEndTrackPos)
    : m_fBeginTrackPosition{ fBeginTrackPos }
    , m_fEndTrackPosition{ fEndTrackPos }
{
}

void CAnimNotifyState::Free()
{
    CBase::Free();
}

#include "EnginePch.h"
#include "GameInstance.h"
#include "AnimNotify.h"

CAnimNotify::CAnimNotify(_float fTrackPosition)
    : m_fTrackPosition{ fTrackPosition }
{
}

void CAnimNotify::Free()
{
    CBase::Free();
}
#pragma once

#ifdef TRACY_ENABLE
#include "Tracy/tracy/Tracy.hpp"

#define PROFILE_FRAME()         FrameMark
#define PROFILE_ZONE()          ZoneScoped
#define PROFILE_ZONE_N(name)    ZoneScopedN(name)
#define PROFILE_THREAD(name)    tracy::SetThreadName(name)

#else

#define PROFILE_FRAME()
#define PROFILE_ZONE()
#define PROFILE_ZONE_N(name)
#define PROFILE_THREAD(name)

#endif

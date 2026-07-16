#pragma once
// Tracy profiling integration
// When AIMIDI_USE_TRACY is defined, include Tracy and use ZoneScoped
// Otherwise, define empty macros

#ifdef AIMIDI_USE_TRACY
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#else
#define ZoneScoped
#define ZoneScopedN(name)
#define FrameMark
#define TracyPlot(name, value)
#endif

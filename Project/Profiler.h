// @Profiler.h
// @brief
// @author ICE
// @date 2023/04/26
//
// @details

#pragma once

#if BE_ENABLE_PROFILING
#include <optick.h>
#endif

#if BE_ENABLE_PROFILING
#define BE_PROFILE_FRAME(...) OPTICK_FRAME(__VA_ARGS__)
#define BE_PROFILE_FUNC(...) OPTICK_EVENT(__VA_ARGS__)
#define BE_PROFILE_TAG(NAME, ...) OPTICK_TAG(NAME, __VA_ARGS__)
#define BE_PROFILE_SCOPE_DYNAMIC(NAME) OPTICK_EVENT_DYNAMIC(NAME)
#define BE_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#else
#define BE_PROFILE_FRAME(...)
#define BE_PROFILE_FUNC(...)
#define BE_PROFILE_TAG(NAME, ...)
#define BE_PROFILE_SCOPE_DYNAMIC(NAME)
#define BE_PROFILE_THREAD(...)
#endif
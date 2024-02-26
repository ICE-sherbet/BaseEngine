// @Profiler.h
// @brief
// @author ICE
// @date 2023/04/26
//
// @details

#pragma once

#if BE_ENABLE_PROFILING
#include <tracy/Tracy.hpp>
#endif

#if BE_ENABLE_PROFILING
#define BE_PROFILE_MARK_FRAME FrameMark;
#define HZ_PROFILE_FUNC(...) ZoneScoped
#define BE_PROFILE_FUNC(...) ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
#define BE_PROFILE_SCOPE(...) BE_PROFILE_FUNC(__VA_ARGS__)
#define BE_PROFILE_SCOPE_DYNAMIC(NAME) ZoneScoped; ZoneName(NAME, strlen(NAME))
#define BE_PROFILE_THREAD(...) tracy::SetThreadName(__VA_ARGS__)
#else
#define BE_PROFILE_MARK_FRAME
#define BE_PROFILE_FUNC(...)
#define BE_PROFILE_SCOPE(...)
#define BE_PROFILE_SCOPE_DYNAMIC(NAME)
#define BE_PROFILE_THREAD(...)
#endif
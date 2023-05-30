// @Assert.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once

#include "Log.h"

#ifdef BE_PLATFORM_WINDOWS
#define BE_DEBUG_BREAK __debugbreak()
#else
#define BE_DEBUG_BREAK
#endif

#ifdef BE_DEBUG
#define BE_ENABLE_ASSERTS
#endif

#define BE_ENABLE_VERIFY

#ifdef BE_ENABLE_ASSERTS
#define BE_CORE_ASSERT_MESSAGE_INTERNAL(...)                              \
  ::base_engine::Log::PrintAssertMessage(::base_engine::Log::Type::kCore, \
                                         "Assertion Failed", __VA_ARGS__)
#define BE_ASSERT_MESSAGE_INTERNAL(...)                                     \
  ::base_engine::Log::PrintAssertMessage(::base_engine::Log::Type::kClient, \
                                         "Assertion Failed", __VA_ARGS__)

#define BE_CORE_ASSERT(condition, ...)              \
  {                                                 \
    if (!(condition)) {                             \
      BE_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
      BE_DEBUG_BREAK;                               \
    }                                               \
  }
#define BE_ASSERT(condition, ...)              \
  {                                            \
    if (!(condition)) {                        \
      BE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); \
      BE_DEBUG_BREAK;                          \
    }                                          \
  }
#else
#define BE_CORE_ASSERT(condition, ...)
#define BE_ASSERT(condition, ...)
#endif

#ifdef BE_ENABLE_VERIFY
#define BE_CORE_VERIFY_MESSAGE_INTERNAL(...)                              \
  ::base_engine::Log::PrintAssertMessage(::base_engine::Log::Type::kCore, \
                                         "Verify Failed", __VA_ARGS__)
#define BE_VERIFY_MESSAGE_INTERNAL(...)                                     \
  ::base_engine::Log::PrintAssertMessage(::base_engine::Log::Type::kClient, \
                                         "Verify Failed", __VA_ARGS__)

#define BE_CORE_VERIFY(condition, ...)              \
  {                                                 \
    if (!(condition)) {                             \
      BE_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); \
      BE_DEBUG_BREAK;                               \
    }                                               \
  }
#define BE_VERIFY(condition, ...)              \
  {                                            \
    if (!(condition)) {                        \
      BE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); \
      BE_DEBUG_BREAK;                          \
    }                                          \
  }
#else
#define BE_CORE_VERIFY(condition, ...)
#define BE_VERIFY(condition, ...)
#endif

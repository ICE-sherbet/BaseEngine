// @config.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <cstdint>
using IdType = uint32_t;
using id_type = IdType;



#if defined __clang__ || defined __GNUC__
#define BECS_PRETTY_FUNCTION __PRETTY_FUNCTION__
#define BECS_PRETTY_FUNCTION_PREFIX '='
#define BECS_PRETTY_FUNCTION_SUFFIX ']'
#elif defined _MSC_VER
#define BECS_PRETTY_FUNCTION __FUNCSIG__
#define BECS_PRETTY_FUNCTION_PREFIX '<'
#define BECS_PRETTY_FUNCTION_SUFFIX '>'
#endif

constexpr int kBePackedPage = 1024;
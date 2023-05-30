// @MonoGlueMacro.h
// @brief
// @author ICE
// @date 2023/04/19
// 
// @details

#pragma once
#include <mono/metadata/object.h>

#define BE_ADD_INTERNAL_CALL(icall)                                      \
  mono_add_internal_call("BaseEngine_ScriptCore.InternalCalls::" #icall, \
                         (void*)glue::internal_calls::icall)
// @DebugGlue.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "MonoGlueMacro.h"
#include "ObjectEntity.h"
#include "ScriptTypes.h"

namespace base_engine::glue::internal_calls {

void DebugLog(MonoString* message);

struct DebugGlue {
  void operator()() const { BE_ADD_INTERNAL_CALL(DebugLog); }
};
}  // namespace base_engine::glue::internal_calls

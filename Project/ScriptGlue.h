// @ScriptGlue.h
// @brief
// @author ICE
// @date 2023/04/25
//
// @details

#pragma once
#include "MonoGlueMacro.h"
#include "ObjectEntity.h"
#include "ScriptTypes.h"

namespace base_engine::glue::internal_calls {
MonoObject* ScriptGetInstance(uint64_t id);
struct ScriptGlue {
  void operator()() const { BE_ADD_INTERNAL_CALL(ScriptGetInstance); }
};
}  // namespace base_engine::glue::internal_calls
// @ShapesGlue.h
// @brief
// @author ICE
// @date 2023/07/10
//
// @details

#pragma once
#include "MonoGlueMacro.h"
#include "ObjectEntity.h"
#include "ScriptTypes.h"

namespace base_engine::glue::internal_calls {

float CircleGetRadius(uint64_t entity_id);
void CircleSetRadius(uint64_t entity_id, float radius);
struct ShapesGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(CircleGetRadius);
    BE_ADD_INTERNAL_CALL(CircleSetRadius);
 }
};
}  // namespace base_engine::glue::internal_calls

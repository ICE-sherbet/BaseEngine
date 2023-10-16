// @RigidBodyGlue.h
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

void VelocityGetVelocity(uint64_t entity_id, Vector2* out_velocity);
void VelocitySetVelocity(uint64_t entity_id, Vector2* in_velocity);
struct RigidBodyGlue {
  void operator()() const {
  	BE_ADD_INTERNAL_CALL(VelocityGetVelocity);
  	BE_ADD_INTERNAL_CALL(VelocitySetVelocity);
  }
};
}  // namespace base_engine::glue::internal_calls
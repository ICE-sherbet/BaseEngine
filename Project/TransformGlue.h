// @TransformGlue.h
// @brief
// @author ICE
// @date 2023/04/17
//
// @details

#pragma once
#include <cstdint>

#include "MonoGlueMacro.h"
#include "Vector3.h"

namespace base_engine::glue::internal_calls {

void TransformComponentGetTranslation(uint64_t entity_id,
                                      Vector3* out_translation);
void TransformComponentSetTranslation(uint64_t entity_id,
                                      const Vector3* in_translation);
void TransformComponentGetLocalTranslation(uint64_t entity_id,
                                      Vector3* out_translation);
void TransformComponentSetLocalTranslation(uint64_t entity_id,
                                      const Vector3* in_translation);
void TransformComponentGetLocalEulerAngles(uint64_t entity_id,
                                           Vector3* out_euler_angle);
void TransformComponentSetLocalEulerAngles(uint64_t entity_id,
                                           const Vector3* in_euler_angle);

void TransformComponentGetScale(uint64_t entity_id, Vector3* out_scale);
void TransformComponentSetScale(uint64_t entity_id, const Vector3* in_scale);
struct TransformGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(TransformComponentGetTranslation);
    BE_ADD_INTERNAL_CALL(TransformComponentSetTranslation);
    BE_ADD_INTERNAL_CALL(TransformComponentGetLocalTranslation);
    BE_ADD_INTERNAL_CALL(TransformComponentSetLocalTranslation);

    BE_ADD_INTERNAL_CALL(TransformComponentGetLocalEulerAngles);
    BE_ADD_INTERNAL_CALL(TransformComponentSetLocalEulerAngles);

  	BE_ADD_INTERNAL_CALL(TransformComponentGetScale);
    BE_ADD_INTERNAL_CALL(TransformComponentSetScale);
  }
};
}  // namespace base_engine::glue::internal_calls

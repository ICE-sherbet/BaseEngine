// @SpriteRendererGlue.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <cstdint>

#include "Asset.h"
#include "MonoGlueMacro.h"

namespace base_engine::glue::internal_calls {

bool SpriteRendererComponentGetTexture(uint64_t entity_id,
                                       AssetHandle* in_handle);
void SpriteRendererComponentSetTexture(uint64_t entity_id,
                                       const AssetHandle* out_handle);

struct SpriteGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(SpriteRendererComponentGetTexture);
    BE_ADD_INTERNAL_CALL(SpriteRendererComponentSetTexture);
  }
};
}  // namespace base_engine::glue::internal_calls

// @TextureGlue.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once
#include <cstdint>

#include "MonoGlueMacro.h"

namespace base_engine::glue::internal_calls {
void Texture2DGetSize(uint64_t* handle, uint64_t* w, uint64_t* h);

struct TextureGlue {
  void operator()() const { BE_ADD_INTERNAL_CALL(Texture2DGetSize); }
};
}  // namespace base_engine::glue::internal_calls

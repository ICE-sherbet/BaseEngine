// @InputGlue.h
// @brief
// @author ICE
// @date 2023/04/18
//
// @details

#pragma once
#include <cstdint>

#include "MonoGlueMacro.h"

namespace base_engine::glue::internal_calls {

bool InputGetKeyDown(int16_t key);
bool InputGetKey(int16_t key);
bool InputGetKeyUp(int16_t key);

struct InputGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(InputGetKeyDown);
    BE_ADD_INTERNAL_CALL(InputGetKey);
    BE_ADD_INTERNAL_CALL(InputGetKeyUp);
  }
};
}  // namespace base_engine::glue::internal_calls

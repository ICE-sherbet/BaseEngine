// @Matrix44.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once

#include "Math.h"
#if defined(_USE_MOF_LIBRARY_)

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Matrix44 = Mof::Matrix44;
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {
using Matrix44 = glm::mat4x4;
}  // namespace base_engine

#endif
// @Matrix33.h
// @brief
// @author ICE
// @date 2023/03/19
// 
// @details

#pragma once
#include "Math.h"

#if defined(_USE_MOF_LIBRARY_)

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Matrix33 = Mof::Matrix33;
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {
using Matrix33 = glm::mat3x3;
}  // namespace base_engine

#endif

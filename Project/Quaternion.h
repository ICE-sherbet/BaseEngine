// @Quaternion.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "Math.h"

#ifdef _USE_MOF_LIBRARY_

namespace base_engine {
// TODO Mofに依存してるから独自クラスにする
using Quaternion = Mof::Quaternion;
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {
using Quaternion = glm::quat;
}  // namespace base_engine

#endif

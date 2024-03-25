// @Vector4.h
// @brief
// @author ICE
// @date 2023/03/22
//
// @details

#pragma once
#include "Math.h"

#ifdef _USE_MOF_LIBRARY_

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Floating = MofFloat;
using Vector4 = Mof::Vector4;
using Color = Vector4;

using Vector4Impl = Mof::Vector4Impl;

using InVector4 = const Vector4&;
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Vector4 = glm::vec4;
using Color = Vector4;

using InVector4 = const Vector4&;
}  // namespace base_engine

#endif
#pragma once
#include "Math.h"

#ifdef _USE_MOF_LIBRARY_

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Floating = MofFloat;
using Vector2 = Mof::Vector2;
using Vector2Impl = Mof::Vector2Impl;

using InVector2 = const Vector2&;

constexpr Vector2Impl kUp = {0, 1};
constexpr Vector2Impl kRight = {1, 0};
constexpr Vector2Impl kOne = {1, 1};
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {

using Floating = float;
using Vector2 = glm::vec2;
using InVector2 = const Vector2&;

constexpr Vector2 kUp = {0, 1};
constexpr Vector2 kRight = {1, 0};
constexpr Vector2 kOne = {1, 1};
}  // namespace base_engine

#endif
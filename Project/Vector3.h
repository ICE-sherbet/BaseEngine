#pragma once
#include "Math.h"

#ifdef _USE_MOF_LIBRARY_

namespace base_engine {

// TODO Mofに依存してるから独自クラスにする
using Vector3 = Mof::Vector3;
using Vector3Impl = Mof::Vector3Impl;

using InVector3 = const Vector3&;
namespace Vec3 {

const Vector3 kZero = {0, 0, 0};
const Vector3 kUp = {0, 1, 0};
const Vector3 kOne = {1, 1, 1};
}  // namespace Vec3
}  // namespace base_engine

#elif defined(_USE_GLM_LIBRARY_)

namespace base_engine {

using Vector3 = glm::vec3;
using InVector3 = const Vector3&;
namespace Vec3 {

constexpr Vector3 kZero = {0, 0, 0};
constexpr Vector3 kUp = {0, 1, 0};
constexpr Vector3 kOne = {1, 1, 1};
}  // namespace Vec3
}  // namespace base_engine

#endif
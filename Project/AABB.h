// @AABB.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include <glm/vec3.hpp>

namespace base_engine {
struct AABB {
  glm::vec3 Min, Max;

  constexpr AABB() : Min(0.0f), Max(0.0f) {}

  constexpr AABB(const glm::vec3& min, const glm::vec3& max)
      : Min(min), Max(max) {}
};
}  // namespace base_engine

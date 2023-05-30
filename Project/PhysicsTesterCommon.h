// @PhysicsTesterCommon.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "Vector2.h"
#include "VectorUtilities.h"

namespace base_engine::physics {
class PhysicsTesterCommon {
 public:
  PhysicsTesterCommon() = delete;

  static bool CircleCircle(const float radius_a, const Vector2& center_a,
                           const float radius_b, const Vector2& center_b,
                           float& penetration, Vector2& normal,
                           Vector2& point) {
    const auto direction = center_a - center_b;

    const auto direction_length_squared =
        VectorUtilities::MagnitudeSquared(direction);
    const auto sum_radii = radius_a + radius_b;
    if (direction_length_squared > sum_radii * sum_radii) return false;

    const auto direction_length = std::sqrt(direction_length_squared);
    penetration = sum_radii - direction_length;
    normal = direction / direction_length;
    point = center_a - direction * 0.5f;
    return true;
  }
};
}  // namespace base_engine::physics

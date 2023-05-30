// @BoundingBox.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "ShapeComponents.h"
#include "Vector2.h"

namespace base_engine::physics {

struct BoundingBox {
  Vector2 min;
  Vector2 max;

  BoundingBox(const Vector2& min, const Vector2& max) : min{min}, max{max} {}
  BoundingBox(const float min_x, const float min_y, const float max_x,
              const float max_y)
      : min{min_x, min_y}, max{max_x, max_y} {}

  static BoundingBox ConstructBoundingBox(const Vector2& pos, const float radius) {
    const auto radius_size = Vector2(radius, radius);
    return {pos - radius_size, pos + radius_size};
  }
};
}  // namespace base_engine::physics
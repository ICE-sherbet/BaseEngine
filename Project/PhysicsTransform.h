// @PhysicsTransform.h
// @brief
// @author かき氷氷味
// @date 2022/10/16
//
// @details

#pragma once
#include "PhysicsRotate.h"
#include "PhysicsVector2.h"

namespace base_engine::physics {
struct PhysicsTransform {
  PhysicsTransform() = default;

  PhysicsTransform(const PVec2& position, const PRot& rotation)
      : p(position), q(rotation) {}

  void SetIdentity() {
    p.SetZero();
    q.SetIdentity();
  }

  void Set(const PVec2& position, const float angle) {
    p = position;
    q.Set(angle);
  }

  PVec2 p;
  PRot q;
};
}  // namespace base_engine::physics

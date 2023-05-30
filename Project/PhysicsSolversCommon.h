// @PhysicsSolversCommon.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "RigidBodyComponent.h"
#include "TransformComponent.h"
#include "Vector2.h"
#include "VelocityComponent.h"

namespace base_engine::physics {
class PhysicsSolversCommon {
 public:
  PhysicsSolversCommon() = delete;
  static void ApplyImpulseBothBodies(const Vector2& normal,
                                     const Vector2& point, float restitution,
                                     const RigidBodyComponent& rigidBodyA,
                                     const RigidBodyComponent& rigidBodyB,
                                     const component::TransformComponent& poseA,
                                     const component::TransformComponent& poseB,
                                     VelocityComponent& velocityA,
                                     VelocityComponent& velocityB) {}
};
}  // namespace base_engine::physics

// @PhysicsCircleShape.h
// @brief
// @author ICE
// @date 2022/10/17
//
// @details

#pragma once
#include <numbers>

#include "PhysicsBlockAllocator.h"
#include "PhysicsShapes.h"

namespace base_engine::physics {
using int32_t = int32_t;
class b2CircleShape : public PhysicsShape {
 public:
  b2CircleShape();

  /// Implement b2Shape.
  PhysicsShape* Clone(PhysicsBlockAllocator* allocator) const override;

  /// @see b2Shape::GetChildCount
  int32_t GetChildCount() const override;

  /// Implement b2Shape.
  bool TestPoint(const PhysicsTransform& transform, const PVec2& p) const override;

  /// Implement b2Shape.
  /// @note because the circle is solid, rays that start inside do not hit
  /// because the normal is not defined.
  bool RayCast(PhysicsRayCastOutput* output, const PhysicsRayCastInput& input,
               const PhysicsTransform& transform, int32_t childIndex) const override;

  /// @see b2Shape::ComputeAABB
  void ComputeAABB(PhysicsAABB* aabb, const PhysicsTransform& transform,
                   int32_t childIndex) const override;

  /// @see b2Shape::ComputeMass
  void ComputeMass(PhysicsMassData* massData, float density) const override;

  /// Position
  PVec2 m_p;
};
}  // namespace base_engine::physics

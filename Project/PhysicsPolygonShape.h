// @PhysicsPolygonShape.h
// @brief
// @author ICE
// @date 2022/10/18
//
// @details

#pragma once
#include "PhysicsComponentData.h"
#include "PhysicsShapes.h"
#include "PhysicsVector2.h"

namespace base_engine::physics {
class PhysicsPolygonShape : public PhysicsShape {
 public:
  PhysicsPolygonShape();

  PhysicsShape* Clone(PhysicsBlockAllocator* allocator) const override;
  
  [[nodiscard]] int32_t GetChildCount() const override;

  void Set(const PVec2* points, uint8_t count);

  void SetAsBox(const float hx, const float hy) {
    m_count = 4;
    m_vertices[0].Set(0, 0);
    m_vertices[1].Set(hx, 0);
    m_vertices[2].Set(hx, hy);
    m_vertices[3].Set(0, hy);
    m_normals[0].Set(0.0f, -1.0f);
    m_normals[1].Set(1.0f, 0.0f);
    m_normals[2].Set(0.0f, 1.0f);
    m_normals[3].Set(-1.0f, 0.0f);
    m_centroid.SetZero();
  }

  void SetAsRect(const float left, const float top, const float right,
                 const float bottom) {
    m_count = 4;
    m_vertices[0].Set(left, top);
    m_vertices[1].Set(right, top);
    m_vertices[2].Set(right, bottom);
    m_vertices[3].Set(left, bottom);
    m_normals[0].Set(0.0f, -1.0f);
    m_normals[1].Set(1.0f, 0.0f);
    m_normals[2].Set(0.0f, 1.0f);
    m_normals[3].Set(-1.0f, 0.0f);
    m_centroid.SetZero();
  }

  bool TestPoint(const PhysicsTransform& transform, const PVec2& p) const override;

  bool RayCast(PhysicsRayCastOutput* output, const PhysicsRayCastInput& input,
               const PhysicsTransform& transform, int32_t childIndex) const override;

  void ComputeAABB(PhysicsAABB* aabb, const PhysicsTransform& transform,
                   int32_t childIndex) const override;

  /// @see b2Shape::ComputeMass
  void ComputeMass(PhysicsMassData* massData, float density) const override {}

  /// Validate convexity. This is a very time consuming operation.
  /// @returns true if valid
  bool Validate() const;

  PVec2 m_centroid{};
  PVec2 m_vertices[kMaxPolygonVertices]{};
  PVec2 m_normals[kMaxPolygonVertices]{};
  int32_t m_count;
};
}  // namespace base_engine::physics

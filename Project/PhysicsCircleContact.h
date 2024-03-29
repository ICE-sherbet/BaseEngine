﻿// @PhysicsCircleContact.h
// @brief
// @author ICE
// @date 2022/10/21
//
// @details

#pragma once
#include "PhysicsContact.h"
namespace base_engine::physics {
class PhysicsCircleContact : PhysicsContact {
 public:
  static PhysicsContact* Create(PhysicsFixture* fixtureA, int32_t indexA,
                           PhysicsFixture* fixtureB, int32_t indexB,
                           PhysicsBlockAllocator* allocator);

  static void Destroy(PhysicsContact* contact, PhysicsBlockAllocator* allocator);

  PhysicsCircleContact(PhysicsFixture* fixtureA, PhysicsFixture* fixtureB);
  ~PhysicsCircleContact();

  void Evaluate(b2Manifold* manifold, const PhysicsTransform& xfA,
                const PhysicsTransform& xfB) override;
};
}  // namespace base_engine::physics

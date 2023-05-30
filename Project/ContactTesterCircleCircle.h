// @ContactTesterCircleCircle.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "ContactTesterSystem.h"
#include "PhysicsTesterCommon.h"
#include "TransformComponent.h"

namespace base_engine::physics {
class ContactTesterCircleCircle
    : public ContactTesterSystem<physics::Circle, physics::Circle> {
 public:
  ContactTesterCircleCircle(const Ref<Scene>& scene,
                            const Ref<PhysicsEngineData>& engine);

 protected:
  void TestPair(Contact& possible_contact) override;
  };
}  // namespace base_engine::physics

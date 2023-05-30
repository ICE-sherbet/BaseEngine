// @PhysicsContactListenerSystem.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"

namespace base_engine::physics {
class PhysicsContactListenerSystem
    : public AbstractPhysicsSystem<PhysicsContactListenerSystem> {
 public:
  PhysicsContactListenerSystem(const Ref<Scene>& scene,
                               const Ref<PhysicsEngineData>& engine)
      : AbstractPhysicsSystem<PhysicsContactListenerSystem>(*this, scene,
                                                            engine) {}

  void OnInit() override {}
  void OnUpdate() override;
};
}  // namespace base_engine::physics

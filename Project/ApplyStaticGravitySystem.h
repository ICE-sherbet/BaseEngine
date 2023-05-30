// @ApplyStaticGravitySystem.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"
namespace base_engine::physics {
class ApplyStaticGravitySystem
    : public AbstractPhysicsSystem<ApplyStaticGravitySystem> {
 public:
  explicit ApplyStaticGravitySystem(const Ref<Scene>& scene,
                                    const Ref<PhysicsEngineData> engine)
      : AbstractPhysicsSystem<ApplyStaticGravitySystem>(*this, scene, engine) {}

  void OnInit() override {}
  void OnUpdate() override;
};
}  // namespace base_engine::physics

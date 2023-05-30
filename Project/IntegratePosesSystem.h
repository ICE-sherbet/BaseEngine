// @IntegratePosesSystem.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"

namespace base_engine::physics {
class IntegratePosesSystem : public AbstractPhysicsSystem<IntegratePosesSystem> {
 public:
  explicit IntegratePosesSystem(const Ref<Scene>& scene,
                                const Ref<PhysicsEngineData> engine);


  void OnInit() override;

  void OnUpdate() override;
private:
};
}  // namespace base_engine::physics

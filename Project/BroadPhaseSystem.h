// @BroadPhaseSystem.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"

namespace base_engine::physics {
class BroadPhaseSystem : public AbstractPhysicsSystem<BroadPhaseSystem> {
 public:
  explicit BroadPhaseSystem(const Ref<Scene>& scene,
                            const Ref<PhysicsEngineData> engine)
      : AbstractPhysicsSystem<BroadPhaseSystem>(*this, scene, engine) {}

  void OnInit() override;
  void OnUpdate() override;

 private:
  void SortBoundingBoxes();
  std::array<float, 128> filter_sort_pool_;
};
}  // namespace base_engine::physics

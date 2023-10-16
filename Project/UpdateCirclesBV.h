// @UpdateCirclesBV.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once
#include "ShapeComponents.h"
#include "UpdateBV.h"

namespace base_engine::physics {
class UpdateCirclesBV : public UpdateBV<physics::CircleShape> {
 public:
  explicit UpdateCirclesBV(const Ref<Scene>& scene,
                           const Ref<PhysicsEngineData> engine)
      : UpdateBV<CircleShape>(scene,engine) {}

  void Process(becs::Entity entity,const Vector3& pos, const CircleShape& shape,
               BoundingBox& box) override;
};
}  // namespace base_engine::physics

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
class UpdateCirclesBV : public UpdateBV<physics::Circle> {
 public:
  explicit UpdateCirclesBV(const Ref<Scene>& scene,
                           const Ref<PhysicsEngineData> engine)
      : UpdateBV<Circle>(scene,engine) {}

  void Process(becs::Entity entity,const Vector3& pos, const Circle& shape,
               BoundingBox& box) override;
};
}  // namespace base_engine::physics

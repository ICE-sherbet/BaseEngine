// @UpdateBV.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "AbstractPhysicsSystem.h"
#include "BoundingBox.h"
#include "Profiler.h"

namespace base_engine::physics {
template <typename Shape>
class UpdateBV : public AbstractPhysicsSystem<UpdateBV<Shape>> {
 public:
  explicit UpdateBV(const Ref<Scene>& scene,
                    const Ref<PhysicsEngineData> engine)
      : AbstractPhysicsSystem<UpdateBV<Shape>>(*this, scene, engine) {}

  void OnInit() override {}
  void OnUpdate() override {
    BE_PROFILE_FUNC("UpdateBV");

    for (const auto shapes =
             this->GetScene()
                 ->template GetAllEntitiesWith<
                     Shape, component::TransformComponent, BoundingBox>();
         auto entity : shapes) {
      const auto& shape = shapes.template get<Shape>(entity);
      const auto pos =
          shapes.template get<component::TransformComponent>(entity)
              .GetGlobalTranslation();
      auto& box = shapes.template get<BoundingBox>(entity);
      Process(entity, pos, shape, box);
    }
  }
  virtual void Process(becs::Entity entity, const Vector3& pos,
                       const Shape& shape, BoundingBox& box) = 0;
};

}  // namespace base_engine::physics

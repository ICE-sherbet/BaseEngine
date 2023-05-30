// @AbstractPhysicsSystem.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "IBecsSystem.h"
#include "ObjectEntity.h"
#include "PhysicsEngineData.h"
#include "Scene.h"

namespace base_engine::physics {

template <typename System>
class AbstractPhysicsSystem : public ISystem {
  struct Context {
    Ref<Scene> scene;
    Ref<PhysicsEngineData> physics_engine;
  };
  Context context_;

 protected:
  explicit AbstractPhysicsSystem(System& system, const Ref<Scene>& scene,
                                 const Ref<PhysicsEngineData> engine)
      : context_(scene, engine), system_(system) {}

  Ref<Scene> GetScene() { return context_.scene; }
  Ref<PhysicsEngineData> GetEngineData() { return context_.physics_engine; }
 private:
  System& system_;
};
}  // namespace base_engine::physics

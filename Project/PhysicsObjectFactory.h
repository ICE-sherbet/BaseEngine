// @PhysicsObjectFactory.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "ObjectEntity.h"
#include "Scene.h"
#include "Vector2.h"

namespace base_engine::physics {
class PhysicsObjectFactory {
 public:
  PhysicsObjectFactory(const Ref<Scene>& scene):scene_context_(scene){}
  static ObjectEntity CreateCircle(ObjectEntity entity, const Vector2& center, float radius, float mass, float restitution);
  static ObjectEntity CreateCircle(ObjectEntity entity, float radius, float mass, float restitution);

private:
  Ref<Scene> scene_context_;
};
}  // namespace base_engine::physics
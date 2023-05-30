#include "PhysicsObjectFactory.h"

#include "BodyMask.h"
#include "BodyTypeTag.h"
#include "BoundingBox.h"
#include "RigidBodyComponent.h"
#include "ShapeComponents.h"
#include "VelocityComponent.h"

namespace base_engine::physics {
ObjectEntity PhysicsObjectFactory::CreateCircle(ObjectEntity entity,
                                                const Vector2& center,
                                                float radius, float mass,
                                                float restitution) {
  const auto mmoi = Circle::CalculateMMOI(radius, mass);
  entity.GetComponent<component::TransformComponent>().SetLocalTranslation(
      {center.x, center.y, 0});

  entity.AddComponent<RigidBodyComponent>(restitution, mass, mmoi);
  entity.AddComponent<VelocityComponent>();
  entity.AddComponent<BodyMask>(Circle::Type(), BodyTypeTag<Dynamic>::Type());
  entity.AddComponent<Circle>().radius = radius;
  entity.AddComponent<BoundingBox>(0, 0, 0, 0);
  return entity;
}

ObjectEntity PhysicsObjectFactory::CreateCircle(ObjectEntity entity,
                                                float radius, float mass,
                                                float restitution) {
  const auto mmoi = Circle::CalculateMMOI(radius, mass);

  entity.AddComponent<RigidBodyComponent>(restitution, mass, mmoi);
  entity.AddComponent<VelocityComponent>();
  entity.AddComponent<BodyMask>(Circle::Type(), BodyTypeTag<Dynamic>::Type());
  if (!entity.HasComponent<Circle>())
    entity.AddComponent<Circle>().radius = radius;
  entity.AddComponent<BoundingBox>(0, 0, 0, 0);
  return entity;
}
}  // namespace base_engine::physics

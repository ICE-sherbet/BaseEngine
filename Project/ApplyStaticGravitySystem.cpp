#include "ApplyStaticGravitySystem.h"

#include "RigidBodyComponent.h"
#include "VelocityComponent.h"

namespace base_engine::physics {
void ApplyStaticGravitySystem::OnUpdate() {
	const Vector2 gravity = Vector2{0, 0} * 5;
  for (const auto bodies =
           GetScene()
               ->GetAllEntitiesWith<RigidBodyComponent, VelocityComponent>();
       const auto& body : bodies) {
    const auto& rigid_body = bodies.get<RigidBodyComponent>(body);
    auto& [linear, force, angular, torque] =
        bodies.get<VelocityComponent>(body);
    force = gravity * rigid_body.mass;
  }
}
}  // namespace base_engine::physics

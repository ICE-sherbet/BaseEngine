#include "RigidBodyGlue.h"

#include "EntityGlue.h"
#include "RigidBodyComponent.h"
#include "VelocityComponent.h"

namespace base_engine::glue::internal_calls {
void VelocityGetVelocity(uint64_t entity_id, Vector2* out_velocity) {
  auto entity = GetEntity(entity_id);
  if (!entity.HasComponent<physics::VelocityComponent>()) {
    return;
  }
  *out_velocity = entity.GetComponent<physics::VelocityComponent>().linear;
}

void VelocitySetVelocity(uint64_t entity_id, Vector2* in_velocity)
{
  auto entity = GetEntity(entity_id);
  if (!entity.HasComponent<physics::VelocityComponent>()) {
    return;
  }
  entity.GetComponent<physics::VelocityComponent>().linear = *in_velocity;
}
}  // namespace base_engine::glue::internal_calls

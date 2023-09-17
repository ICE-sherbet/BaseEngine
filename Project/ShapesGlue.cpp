#include "ShapesGlue.h"

#include "EntityGlue.h"
#include "ShapeComponents.h"
namespace base_engine::glue::internal_calls {
float CircleGetRadius(uint64_t entity_id) {
  auto entity = GetEntity(entity_id);
  if (!entity.HasComponent<physics::CircleShape>()) {
    return 0.0f;
  }
  return entity.GetComponent<physics::CircleShape>().radius;
}

void CircleSetRadius(uint64_t entity_id, float radius) {
  auto entity = GetEntity(entity_id);
  if (!entity.HasComponent<physics::CircleShape>()) {
    return;
  }
  entity.GetComponent<physics::CircleShape>().radius = radius;
}
}  // namespace base_engine::glue::internal_calls

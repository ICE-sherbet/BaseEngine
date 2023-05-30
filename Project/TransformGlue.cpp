#include "TransformGlue.h"

#include "EntityGlue.h"

namespace base_engine::glue::internal_calls {
void TransformComponentGetTranslation(const uint64_t entity_id,
                                      Vector3* out_translation) {
  auto entity = GetEntity(entity_id);

  *out_translation = entity.GetComponent<component::TransformComponent>()
                         .GetGlobalTranslation();
}

void TransformComponentSetTranslation(const uint64_t entity_id,
                                      const Vector3* in_translation) {
  auto entity = GetEntity(entity_id);
  entity.GetComponent<component::TransformComponent>().SetGlobalTranslation(
      *in_translation);
}
void TransformComponentGetLocalTranslation(const uint64_t entity_id,
                                      Vector3* out_translation) {
  auto entity = GetEntity(entity_id);

  *out_translation = entity.GetComponent<component::TransformComponent>()
                         .GetLocalTranslation();
}

void TransformComponentSetLocalTranslation(const uint64_t entity_id,
                                      const Vector3* in_translation) {
  auto entity = GetEntity(entity_id);
  entity.GetComponent<component::TransformComponent>().SetLocalTranslation(
      *in_translation);
}

void TransformComponentGetLocalEulerAngles(uint64_t entity_id,
                                           Vector3* out_euler_angle) {
  auto entity = GetEntity(entity_id);

  *out_euler_angle =
      entity.GetComponent<component::TransformComponent>().GetLocalRotationEuler();
}

void TransformComponentSetLocalEulerAngles(uint64_t entity_id,
                                           const Vector3* in_euler_angle) {
  auto entity = GetEntity(entity_id);

  entity.GetComponent<component::TransformComponent>().SetLocalRotationEuler(
      *in_euler_angle);
}

void TransformComponentGetScale(uint64_t entity_id, Vector3* out_scale) {
  auto entity = GetEntity(entity_id);

  *out_scale = entity.GetComponent<component::TransformComponent>().GetLocalScale();
}

void TransformComponentSetScale(uint64_t entity_id, const Vector3* in_scale) {
  auto entity = GetEntity(entity_id);

  entity.GetComponent<component::TransformComponent>().SetLocalScale(*in_scale);
}
}  // namespace base_engine::glue::internal_calls
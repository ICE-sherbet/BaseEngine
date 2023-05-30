#include "SpriteRendererGlue.h"

#include "EntityGlue.h"
#include "SpriteRendererComponent.h"

namespace base_engine::glue::internal_calls {
bool SpriteRendererComponentGetTexture(const uint64_t entity_id,
                                       AssetHandle* in_handle) {
  auto entity = GetEntity(entity_id);
  const auto& sprite =
      entity.GetComponent<component::SpriteRendererComponent>();
  *in_handle = sprite.texture;
  return true;
}

void SpriteRendererComponentSetTexture(const uint64_t entity_id,
                                       const AssetHandle* out_handle) {
  auto entity = GetEntity(entity_id);
  auto& sprite = entity.GetComponent<component::SpriteRendererComponent>();
  sprite.texture = *out_handle;
}
}  // namespace base_engine::glue::internal_calls
#include "TextureGlue.h"

#include "AssetManager.h"
#include "Texture.h"

namespace base_engine::glue::internal_calls {
void Texture2DGetSize(uint64_t* handle, uint64_t* w, uint64_t* h) {
  auto texture = AssetManager::GetAsset<Texture>(*handle);
  *w = texture->GetWidth();
  *h = texture->GetHeight();
}
}  // namespace base_engine::glue::internal_calls
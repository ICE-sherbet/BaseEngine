#include "TextureGlue.h"

#include "AssetManager.h"
#include "MofTexture.h"

namespace base_engine::glue::internal_calls {
void Texture2DGetSize(uint64_t* handle, uint64_t* w, uint64_t* h) {
  auto texture = AssetManager::GetAsset<MofTexture>(*handle);
  *w = texture->texture_->GetWidth();
  *h = texture->texture_->GetHeight();
}
}  // namespace base_engine::glue::internal_calls
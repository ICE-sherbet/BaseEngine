#include "EditorTextureResource.h"

namespace base_engine::editor {
Ref<Texture> LoadTexture(const std::filesystem::path& relative_path) {
  const auto path = std::filesystem::path("Editor") / relative_path;

  if (!std::filesystem::exists(path)) {
    BE_CORE_ASSERT(false, "Texture not found.");
    return nullptr;
  }

  return TextureUtility::Create(path);
}

Ref<Texture> ThemeDB::GetIcon(const std::string& name) {
  return icon_resource_.GetIcon(name);
}
ThemeDB* ThemeDB::instance_ = nullptr;

ThemeDB::ThemeDB() { icon_resource_.Initialize(); }
void EditorTextureResource::Initialize() {
  auto IconLoad = [this](const std::string& key,
                         const std::filesystem::path& relative_path) {
    icon_map_[key] = LoadTexture(relative_path);
  };
  IconLoad("DefaultFolder", "DefaultFolder.png");
  IconLoad("DefaultAssetItem", "DefaultAssetItem.png");
}

Ref<Texture> EditorTextureResource::GetIcon(const std::string& name) {
  if (!icon_map_.contains(name)) {
    BE_CORE_ERROR("テクスチャが見つかりません");
    return nullptr;
  }
  return icon_map_[name];
}
}  // namespace base_engine::editor

#include "EditorTextureResource.h"

#include "imgui.h"

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

uint32_t ThemeDB::GetColor(const std::string& name) {
  return color_resource_.GetColor(name);
}

ThemeDB* ThemeDB::instance_ = nullptr;

ThemeDB::ThemeDB()
{
	icon_resource_.Initialize();
  color_resource_.Initialize();
}
void EditorTextureResource::Initialize() {
  auto IconLoad = [this](const std::string& key,
                         const std::filesystem::path& relative_path) {
    icon_map_[key] = LoadTexture(relative_path);
  };

  IconLoad("DefaultFolder", "DefaultFolder.png");
  IconLoad("DefaultAssetItem", "DefaultAssetItem.png");
  IconLoad("BackIcon", "BackArrow.png");
  IconLoad("ForwardIcon", "ForwardArrow.png");
}

Ref<Texture> EditorTextureResource::GetIcon(const std::string& name) {
  if (!icon_map_.contains(name)) {
    BE_CORE_ERROR("テクスチャが見つかりません");
    return nullptr;
  }
  return icon_map_[name];
}

void EditorColorResource::Initialize() {
  auto ColorLoad = [this](const std::string& key, const uint32_t col) {
    color_map_[key] = col;
  };

  ColorLoad("BackGroundDark", IM_COL32(26, 26, 26, 255));
  ColorLoad("TextDarker", IM_COL32(128, 128, 128, 255));
}

uint32_t EditorColorResource::GetColor(const std::string& name) {
  if (!color_map_.contains(name)) {
    BE_CORE_ERROR("カラースキームが見つかりません");
    return 0;
  }
  return color_map_[name];
}
}  // namespace base_engine::editor

#include "AssetsBrowserPanel.h"

#include <ranges>

#include "AssetManager.h"
#include "AssetsBrowserItems.h"
#include "AssetsBrowserSetting.h"
#include "EditorTextureResource.h"
#include "imgui.h"

namespace base_engine::editor {
struct AssetsBrowserPanel::BrowserPanelTheme {
  float padding = 2.0f;

  std::unordered_map<std::string, Ref<Texture>> icon_map;
  ~BrowserPanelTheme() = default;
};

AssetsBrowserPanel::AssetsBrowserPanel() {
  theme_ = std::make_unique<BrowserPanelTheme>();
}

void AssetsBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Assets Browser");


  const float paddingForOutline = 2.0f;
  const float scrollBarrOffset = 20.0f + ImGui::GetStyle().ScrollbarSize;
  float panelWidth = ImGui::GetContentRegionAvail().x - scrollBarrOffset;
  float cellSize = AssetsBrowserSetting::Get().thumbnail_size +
                   theme_->padding + paddingForOutline;
  int columnCount = static_cast<int>(panelWidth / cellSize);
  if (ImGui::BeginTable("AssetsBrowserPanelTable", columnCount)) {
    for (auto& item : current_items_) {
      ImGui::TableNextColumn();
      item->OnRenderBegin();
      item->OnRender();
      item->OnRenderEnd();
    }
    ImGui::EndTable();
  }
  ImGui::End();
}

void AssetsBrowserPanel::Initialize(const Ref<Scene>& context) {
  const AssetHandle base_directory_handle = ProcessDirectory(".", nullptr);
  base_directory_ = directories_[base_directory_handle];
  ChangeDirectory(base_directory_);
}

void AssetsBrowserPanel::SetSceneContext(const Ref<Scene>& context) {}

Ref<DirectoryInfo> AssetsBrowserPanel::GetDirectory(
    const std::filesystem::path& filepath) const {
  if (filepath.empty() || filepath.string() == ".") return base_directory_;

  for (const auto& directory : directories_ | std::views::values) {
    if (directory->filepath == filepath) return directory;
  }

  return nullptr;
}

AssetHandle AssetsBrowserPanel::ProcessDirectory(
    const std::filesystem::path& directory_path,
    const Ref<DirectoryInfo>& parent) {
  if (const auto& directory = GetDirectory(directory_path))
    return directory->handle;

  Ref<DirectoryInfo> directory_info = Ref<DirectoryInfo>::Create();
  directory_info->handle = AssetHandle();
  directory_info->parent = parent;

  directory_info->filepath = std::filesystem::relative(directory_path);

  for (const auto& entry :
       std::filesystem::directory_iterator(directory_path)) {
    if (entry.is_directory()) {
      AssetHandle subdir_handle =
          ProcessDirectory(entry.path(), directory_info);
      directory_info->sub_directories[subdir_handle] =
          directories_[subdir_handle];
      continue;
    }

    auto metadata = AssetManager::GetEditorAssetManager()->GetMetadata(
        std::filesystem::relative(entry.path()));
    if (!metadata.IsValid()) {
      if (EditorAssetManager::GetAssetTypeFromPath(entry.path()) ==
          AssetType::kNone)
        continue;

      metadata.handle =
          AssetManager::GetEditorAssetManager()->ImportAsset(entry.path());
    }

    if (!metadata.IsValid()) continue;

    directory_info->assets.push_back(metadata.handle);
  }

  directories_[directory_info->handle] = directory_info;
  return directory_info->handle;
}

void AssetsBrowserPanel::ChangeDirectory(Ref<DirectoryInfo>& directory) {
  if (!directory) return;
  current_items_.Clear();
  for (const auto& directory_info :
       directory->sub_directories | std::views::values) {
    current_items_.items_.emplace_back(
        Ref<ContentBrowserDirectory>::Create(directory_info));
  }
  for (const auto asset_handle : directory->assets) {
    if (AssetMetadata metadata =
            AssetManager::GetEditorAssetManager()->GetMetadata(asset_handle);
        metadata.IsValid()) {
      const auto extension = metadata.file_path.extension().string();
      Ref<Texture> icon;
      if (metadata.type == AssetType::kTexture) {
        icon = AssetManager::GetAsset<Texture>(metadata.handle);
      } else {
        if (theme_->icon_map.contains(extension)) {
          icon = theme_->icon_map[extension];
        } else {
          icon = ThemeDB::GetInstance()->GetIcon("DefaultAssetItem");
        }
      }
      auto item = Ref<ContentBrowserAsset>::Create(metadata, icon);
      current_items_.items_.push_back(item);
    }
  }
}

AssetsBrowserPanel::~AssetsBrowserPanel() = default;
}  // namespace base_engine::editor

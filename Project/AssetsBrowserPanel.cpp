#include "AssetsBrowserPanel.h"

#include <ranges>

#include "AssetManager.h"
#include "AssetsBrowserItems.h"
#include "AssetsBrowserSetting.h"
#include "EditorTextureResource.h"
#include "ImGuiUtilities.h"
#include "Prefab.h"
#include "SelectManager.h"
#include "imgui.h"

namespace base_engine::editor {

class AssetsBrowserPanel::AssetsCreator {
 public:
  AssetsCreator(AssetsBrowserPanel& assets_browser_panel)
      : owner_(assets_browser_panel) {}

  template <std::derived_from<Asset> T, typename... Args>
  Ref<T> CreateAsset(const std::string& filename, Args&&... args) {
    return CreateAssetInDirectory<T>(filename, owner_.current_directory_,
                                     std::forward<Args>(args)...);
  }
  template <std::derived_from<Asset> T, typename... Args>
  Ref<T> CreateAssetInDirectory(const std::string& filename,
                                Ref<DirectoryInfo>& directory, Args&&... args) {
	  const auto filepath = directory->filepath / filename;

    Ref<T> asset = AssetManager::GetEditorAssetManager()->CreateNewAsset<T>(
        filepath.filename().string(), directory->filepath.string(),
        std::forward<Args>(args)...);
    if (!asset) return nullptr;

    directory->assets.push_back(asset->handle_);

    const auto& metadata =
        AssetManager::GetEditorAssetManager()->GetMetadata(asset->handle_);

    return asset;
  }

 private:
  AssetsBrowserPanel& owner_;
};

std::mutex AssetsBrowserPanel::lock_mutex_;

struct AssetsBrowserPanel::BrowserPanelTheme {
  float padding = 2.0f;

  std::unordered_map<std::string, Ref<Texture>> icon_map;
  ~BrowserPanelTheme() = default;
};

AssetsBrowserPanel::AssetsBrowserPanel() {
  theme_ = std::make_unique<BrowserPanelTheme>();
  assets_creator_ = std::make_unique<AssetsCreator>(*this);
}

void AssetsBrowserPanel::OnImGuiRender() {
  ImGui::Begin("Assets Browser");
  RenderTopBar();
  RenderCurrentDirectoryContent();

  ImGui::End();
}

void AssetsBrowserPanel::Initialize(const Ref<Scene>& context) {
  const AssetHandle base_directory_handle = ProcessDirectory(".", nullptr);
  base_directory_ = directories_[base_directory_handle];
  AssetManager::GetEditorAssetManager()->SetAssetChangeCallback(
      [this](auto events) { OnFileSystemChanged(events); });
  ChangeDirectory(base_directory_);
  scene_context_ = context;
}

void AssetsBrowserPanel::SetSceneContext(const Ref<Scene>& context) {
  scene_context_ = context;
}

Ref<DirectoryInfo> AssetsBrowserPanel::GetDirectory(
    const std::filesystem::path& filepath) const {
  if (directories_.empty()) return nullptr;

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
    current_items_.AddItem(
        Ref<AssetsBrowserDirectory>::Create(directory_info));
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
      auto item = Ref<AssetsBrowserAsset>::Create(metadata, icon);
      current_items_.AddItem(item);
    }
  }

  previous_directory_ = directory;
  current_directory_ = directory;
}

void AssetsBrowserPanel::RenderTopBar() {
  const float top_bar_height = 25.0f;

  ImGui::BeginChild("##top_bar", ImVec2(0, top_bar_height * 1.2f), false,
                    ImGuiWindowFlags_NoScrollbar);

  auto content_browser_button = [top_bar_height](const char* labelId,
                                                 Ref<Texture> icon) {
    const ImU32 button_col = ThemeDB::GetInstance()->GetColor("BackGroundDark");
    const ImU32 button_col_p = ui::ColorWithMultipliedValue(button_col, 0.8f);
    ui::ScopedColorStack button_colors(ImGuiCol_Button, button_col,
                                       ImGuiCol_ButtonHovered, button_col,
                                       ImGuiCol_ButtonActive, button_col_p);

    const float icon_size = std::min(24.0f, top_bar_height);
    const float icon_padding = 3.0f;
    const bool clicked = ImGui::Button(labelId, ImVec2(icon_size, icon_size));
    const auto normal_color = ThemeDB::GetInstance()->GetColor("TextDarker");

    ui::DrawButtonImage(
        icon->GetTexture(), normal_color,
        ui::ColorWithMultipliedValue(normal_color, 1.2f),
        ui::ColorWithMultipliedValue(normal_color, 0.8f),
        ui::RectExpanded(ui::GetItemRect(), -icon_padding, -icon_padding));

    return clicked;
  };
  const auto back = ThemeDB::GetInstance()->GetIcon("BackIcon");

  if (content_browser_button("##Back", back)) {
    OnBrowseBack();
  }

  ImGui::SameLine();

  const auto forward = ThemeDB::GetInstance()->GetIcon("ForwardIcon");

  if (content_browser_button("##Forward", forward)) {
    OnBrowseForward();
  }
  ImGui::Separator();

  ImGui::EndChild();
}
std::mutex AssetsBrowserPanel::LockMutex;

void AssetsBrowserPanel::RenderCurrentDirectoryContent() {
  std::scoped_lock<std::mutex> lock(LockMutex);

  constexpr float padding_for_outline = 2.0f;
  const float scroll_barr_offset = 20.0f + ImGui::GetStyle().ScrollbarSize;
  const float panel_width =
      ImGui::GetContentRegionAvail().x - scroll_barr_offset;
  const float cell_size = AssetsBrowserSetting::Get().thumbnail_size +
                          theme_->padding + padding_for_outline;
  const int column_count = static_cast<int>(panel_width / cell_size);
  if (ImGui::BeginTable("AssetsBrowserPanelTable", column_count)) {

    for (auto& item : current_items_) {
      ImGui::TableNextColumn();
      item->OnRenderBegin();
      auto result = item->OnRender();

      if (result.IsSet(AssetsBrowserAction::kSelect)) {
        SelectManager::Instance()->SelectItem("AssetsBrowser",
                                              item->GetHandle());
      }

      item->OnRenderEnd();

      if (result.IsSet(AssetsBrowserAction::kActivated)) {
        if (item->GetType() == AssetsBrowserItem::ItemType::kDirectory) {
          SelectManager::Instance()->SelectItem("AssetsBrowser", 0);
          ChangeDirectory(
              item.As<AssetsBrowserDirectory>()->GetDirectoryInfo());
          break;
        }
      }
    }
    ImGui::EndTable();

    if (ImGui::BeginDragDropTarget()) {
      if (const auto data =
              ImGui::AcceptDragDropPayload("scene_entity_hierarchy")) {
        const size_t count = data->DataSize / sizeof(UUID);

        for (size_t i = 0; i < count; i++) {
          const UUID entity_id = *(static_cast<UUID*>(data->Data) + i);
          auto entity = scene_context_->TryGetEntityWithUUID(entity_id);

          if (!entity) {
            continue;
          }

          Ref<Prefab> prefab = assets_creator_->CreateAsset<Prefab>(
              entity.GetComponent<component::TagComponent>().Tag() + ".prefab");
          prefab->Create(entity);
        }
      }
      ImGui::EndDragDropTarget();
    }
  }
}

void AssetsBrowserPanel::Refresh() {
  std::lock_guard<std::mutex> lock_guard(lock_mutex_);
  RefreshWithoutLock();
}

void AssetsBrowserPanel::RefreshWithoutLock() {
  directories_.clear();
  current_items_.Clear();

  Ref<DirectoryInfo> current_directory = current_directory_;
  const AssetHandle base_directory_handle = ProcessDirectory(".", nullptr);
  base_directory_ = directories_[base_directory_handle];
  current_directory_ = GetDirectory(current_directory->filepath);

  if (!current_directory_)
    current_directory_ = base_directory_;  // Our current directory was removed

  ChangeDirectory(current_directory_);
}

void AssetsBrowserPanel::OnBrowseBack() {
  next_directory_ = current_directory_;
  previous_directory_ = current_directory_->parent;
  ChangeDirectory(previous_directory_);
}

void AssetsBrowserPanel::OnBrowseForward() { ChangeDirectory(next_directory_); }

void AssetsBrowserPanel::OnFileSystemChanged(
    const std::vector<FileSystemChangedEvent>& events) {
  Refresh();

  for (const auto& e : events) {
    if (e.action != FileSystemAction::kAdded) continue;

    AssetHandle handle = 0;

    if (!e.is_directory) {
      if (auto asset =
              AssetManager::GetEditorAssetManager()->GetAsset(e.filepath)) {
        handle = asset->handle_;
      }
    } else {
      const auto& directory_info = GetDirectory(e.filepath);
      handle = directory_info->handle;
    }

    if (handle == 0) continue;

    const size_t item_index = current_items_.FindItem(handle);

    if (item_index == ContentBrowserItemList::kInvalidItem) continue;

    auto& item = current_items_[item_index];
    SelectManager::Instance()->SelectItem("AssetsBrowser", handle);
    break;
  }
}

AssetsBrowserPanel::~AssetsBrowserPanel() = default;
}  // namespace base_engine::editor

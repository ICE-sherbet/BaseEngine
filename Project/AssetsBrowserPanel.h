// @AssetsBrowserPanel.h
// @brief
// @author ICE
// @date 2023/06/12
//
// @details

#pragma once
#include "AssetsBrowserItems.h"
#include "EditorPanel.h"
#include "FileSystem.h"

namespace base_engine::editor {
class AssetsBrowserPanel : public EditorPanel {
 public:
  AssetsBrowserPanel();
  ~AssetsBrowserPanel() override;

  void OnImGuiRender() override;
  void Initialize(const Ref<Scene>& context) override;
  void SetSceneContext(const Ref<Scene>& context) override;

  Ref<DirectoryInfo> GetDirectory(const std::filesystem::path& filepath) const;

 private:
  AssetHandle ProcessDirectory(const std::filesystem::path& directory_path,
                               const Ref<DirectoryInfo>& parent);

  void ChangeDirectory(Ref<DirectoryInfo>& directory);

  void RenderTopBar();
  void RenderCurrentDirectoryContent();

  void Refresh();
  void RefreshWithoutLock();

  void OnBrowseBack();
  void OnBrowseForward();

  void OnFileSystemChanged(const std::vector<FileSystemChangedEvent>& events);

  struct BrowserPanelTheme;
  std::unique_ptr<BrowserPanelTheme> theme_;

  ContentBrowserItemList current_items_;
  Ref<DirectoryInfo> current_directory_;
  Ref<DirectoryInfo> base_directory_;
  Ref<DirectoryInfo> next_directory_, previous_directory_;

  std::unordered_map<AssetHandle, Ref<DirectoryInfo>> directories_;

  static std::mutex lock_mutex_;
};
}  // namespace base_engine::editor

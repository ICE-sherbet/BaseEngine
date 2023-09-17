// @AssetsBrowserItems.h
// @brief
// @author ICE
// @date 2023/06/12
//
// @details

#pragma once
#include <filesystem>

#include "Asset.h"
#include "Ref.h"
#include "Texture.h"

namespace base_engine::editor {
struct DirectoryInfo : public RefCounted {
  AssetHandle handle;

  Ref<DirectoryInfo> parent;

  std::filesystem::path filepath;

  std::vector<AssetHandle> assets;

  using SubDirectoryMap = std::map<AssetHandle, Ref<DirectoryInfo>>;
  SubDirectoryMap sub_directories;
};

enum class AssetsBrowserAction { kNone = 0, kSelect = 1, kActivated = 2 };
struct AssetsBrowserItemActionResult {
  uint16_t field = 0;

  void Set(AssetsBrowserAction flag, bool value) {
    if (value)
      field |= static_cast<uint16_t>(flag);
    else
      field &= ~static_cast<uint16_t>(flag);
  }

  [[nodiscard]] bool IsSet(AssetsBrowserAction flag) const {
    return static_cast<uint16_t>(flag) & field;
  }
};

class AssetsBrowserItem : public RefCounted {
  using ItemActionResult = AssetsBrowserItemActionResult;

 public:
  enum class ItemType : uint16_t { kNone, kDirectory, kAsset };

  AssetsBrowserItem(ItemType type, const AssetHandle& handle,
                    const std::string& name, const Ref<Texture>& icon);

  void OnRenderBegin();
  ItemActionResult OnRender();
  void OnRenderEnd();
  AssetHandle GetHandle() const { return handle_; }
  ItemType GetType() const { return type_; }
  const std::string& GetName() const { return filename_; }

  const Ref<Texture>& GetIcon() const { return icon_; }

 private:
  ItemType type_ = ItemType::kNone;
  AssetHandle handle_;
  Ref<Texture> icon_;
  std::string display_name_;
  std::string filename_;

  bool is_renaming_ = false;
  bool is_dragging_ = false;
  bool just_selected_ = false;
};

class AssetsBrowserDirectory final : public AssetsBrowserItem {
 public:
  explicit AssetsBrowserDirectory(const Ref<DirectoryInfo>& directory_info);

  Ref<DirectoryInfo>& GetDirectoryInfo() { return directory_info_; }

 private:
  Ref<DirectoryInfo> directory_info_;
};
class AssetsBrowserAsset : public AssetsBrowserItem {
 public:
  AssetsBrowserAsset(const AssetMetadata& asset_info, const Ref<Texture>& icon);

  const AssetMetadata& GetAssetInfo() const { return asset_info_; }

 private:
  AssetMetadata asset_info_;
};
class ContentBrowserItemList {
 public:
  static constexpr size_t kInvalidItem = std::numeric_limits<size_t>::max();

  using ItemList = std::vector<Ref<AssetsBrowserItem>>;

  ItemList::iterator begin() { return items_.begin(); }
  ItemList::iterator end() { return items_.end(); }
  ItemList::const_iterator begin() const { return items_.begin(); }
  ItemList::const_iterator end() const { return items_.end(); }

  Ref<AssetsBrowserItem>& operator[](const size_t index) {
    return items_[index];
  }
  const Ref<AssetsBrowserItem>& operator[](const size_t index) const {
    return items_[index];
  }

  ContentBrowserItemList() = default;

  ContentBrowserItemList(const ContentBrowserItemList& other)
      : items_(other.items_) {}

  ContentBrowserItemList& operator=(const ContentBrowserItemList& other) {
    items_ = other.items_;
    return *this;
  }

  void Clear() {
    std::scoped_lock lock(mutex_);
    items_.clear();
  }

  void erase(const AssetHandle handle) {
    const size_t index = FindItem(handle);
    if (index == kInvalidItem) return;

    std::scoped_lock lock(mutex_);
    const auto it = items_.begin() + index;
    items_.erase(it);
  }

  size_t FindItem(const AssetHandle handle) {
    if (items_.empty()) return kInvalidItem;

    std::scoped_lock lock(mutex_);
    for (size_t i = 0; i < items_.size(); i++) {
      if (items_[i]->GetHandle() == handle) return i;
    }

    return kInvalidItem;
  }
  void AddItem(const Ref<AssetsBrowserItem>& item) {
    std::scoped_lock lock(mutex_);
    items_.push_back(item);
  }

 private:
  std::mutex mutex_;
  ItemList items_;
};
}  // namespace base_engine::editor

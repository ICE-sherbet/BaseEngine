// @EditorAssetManager.h
// @brief
// @author ICE
// @date 2023/04/03
//
// @details

#pragma once
#include "AssetImporter.h"
#include "AssetRegistry.h"
#include "ConnectableObject.h"
#include "FileSystem.h"
#include "IBaseEngineAssetManager.h"

namespace base_engine {
class EditorAssetManager final : public IBaseEngineAssetManager,
                                 public ConnectableObject {
 public:
  using AssetsChangeEventFunc =
      std::function<void(const std::vector<FileSystemChangedEvent>&)>;

  EditorAssetManager();

  void Initialize() override;
  ~EditorAssetManager() override;
  AssetType GetAssetType(const AssetHandle asset_handle) override;
  Ref<Asset> GetAsset(AssetHandle asset_handle) override;
  void AddMemoryOnlyAsset(Ref<Asset> asset) override;
  bool ReloadData(AssetHandle asset_handle) override;

  bool IsAssetHandleValid(const AssetHandle asset_handle) override;
  bool IsMemoryAsset(AssetHandle handle) override;
  bool IsAssetLoaded(AssetHandle handle) override;

  std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) override;

  const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() override;

  const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets()
      override;

  Ref<Asset> GetAsset(const std::filesystem::path file_path) override;

  const AssetMetadata& GetMetadata(AssetHandle handle);
  const AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

  void SetAssetChangeCallback(const AssetsChangeEventFunc& callback) {
    AssetsChangeCallback = callback;
  }

  AssetHandle ImportAsset(const std::filesystem::path& filepath);

  static AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
  AssetMetadata& GetMutableMetadata(AssetHandle handle) override;
  const AssetRegistry& GetAssetRegistry() const;

  template <typename T, typename... Args>
  Ref<T> CreateNewAsset(const std::filesystem::path& filename,
                        const std::filesystem::path& directory_path,
                        Args&&... args) {
    AssetMetadata metadata;
    metadata.handle = AssetHandle();
    if (directory_path.empty() || directory_path == ".")
      metadata.file_path = filename;
    else
      metadata.file_path = directory_path / filename;
    metadata.is_data_loaded = true;
    metadata.type = T::GetStaticType();
    if (auto pre_asset = GetAsset(metadata.file_path)) {
      if (pre_asset->GetAssetType() == T::GetStaticType())
        return pre_asset.As<T>();
    }
    asset_registry_[metadata.handle] = metadata;

    WriteRegistryToFile();

    Ref<T> asset = Ref<T>::Create(std::forward<Args>(args)...);
    asset->handle_ = metadata.handle;
    loaded_assets_[asset->handle_] = asset;
    AssetImporter::Serialize(metadata, asset);

    return asset;
  }

 private:
  void LoadAssetRegistry();

  void ReloadAssetFiles();
  void WriteRegistryToFile();
  AssetMetadata& GetMetadataInternal(AssetHandle handle);
  void OnFileSystemChanged(const std::vector<FileSystemChangedEvent>& events);
  void OnAssetRenamed(AssetHandle asset_handle,
                      const std::filesystem::path& new_filepath);
  void OnAssetDeleted(AssetHandle asset_handle);

 private:
  std::unordered_map<AssetHandle, Ref<Asset>> loaded_assets_;
  std::unordered_map<AssetHandle, Ref<Asset>> memory_assets_;

  AssetsChangeEventFunc AssetsChangeCallback;

  AssetRegistry asset_registry_;
};

}  // namespace base_engine

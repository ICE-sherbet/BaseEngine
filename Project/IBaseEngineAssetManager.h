// @IBaseEngineAssetManager.h
// @brief
// @author ICE
// @date 2023/04/03
//
// @details

#pragma once
#include <filesystem>
#include <string_view>
#include <unordered_set>

#include "Asset.h"
#include "AssetMetadata.h"

namespace base_engine {
class IBaseEngineAssetManager {
 public:
  static IBaseEngineAssetManager* Create();

  virtual void Initialize() = 0;
  virtual ~IBaseEngineAssetManager() = default;

  virtual AssetType GetAssetType(AssetHandle asset_handle) = 0;
  virtual Ref<Asset> GetAsset(AssetHandle asset_handle) = 0;
  virtual Ref<Asset> GetAsset(std::filesystem::path file_path) = 0;
  virtual void AddMemoryOnlyAsset(Ref<Asset> asset) = 0;
  virtual bool ReloadData(AssetHandle asset_handle) = 0;
  virtual bool IsAssetHandleValid(AssetHandle asset_handle) = 0;
  virtual bool IsMemoryAsset(AssetHandle handle) = 0;
  virtual bool IsAssetLoaded(AssetHandle handle) = 0;

  virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(
      AssetType type) = 0;
  virtual const std::unordered_map<AssetHandle, Ref<Asset>>&
  GetLoadedAssets() = 0;
  virtual const std::unordered_map<AssetHandle, Ref<Asset>>&
  GetMemoryOnlyAssets() = 0;

  virtual AssetMetadata& GetMutableMetadata(AssetHandle handle) = 0;
};
}  // namespace base_engine

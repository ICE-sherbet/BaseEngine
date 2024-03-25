// @AssetManager.h
// @brief
// @author ICE
// @date 2023/03/29
//
// @details

#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "AssetRegistry.h"
#include "BaseEngineCore.h"
#include "EditorAssetManager.h"
#include "IBaseEngineAssetManager.h"

namespace base_engine {
class AssetManager {
 public:
  static bool IsAssetHandleValid(const AssetHandle uuid) {
    return BASE_ENGINE(AssetManager)->IsAssetHandleValid(uuid);
  }
  template <typename T>
  static Ref<T> GetAsset(const AssetHandle assetHandle) {
    Ref<Asset> asset = BASE_ENGINE(AssetManager)->GetAsset(assetHandle);
    return asset.As<T>();
  }
  template <typename T>
  static Ref<T> GetAsset(const std::filesystem::path path) {
    Ref<Asset> asset = BASE_ENGINE(AssetManager)->GetAsset(path);
    return asset.As<T>();
  }
  static AssetMetadata& GetMutableMetadata(const AssetHandle handle) {
    return BASE_ENGINE(AssetManager)->GetMutableMetadata(handle);
  }
  template <typename TAsset, typename... TArgs>
  static AssetHandle CreateMemoryOnlyAssetWithHandle(AssetHandle handle,
                                                     TArgs&&... args) {
    static_assert(
        std::is_base_of_v<Asset, TAsset>,
        "CreateMemoryOnlyAsset は Asset から派生した型に対してのみ機能します");

    Ref<TAsset> asset = Ref<TAsset>::Create(std::forward<TArgs>(args)...);
    asset->handle_ = handle;

    BASE_ENGINE(AssetManager)->AddMemoryOnlyAsset(asset);
    return asset->handle_;
  }

  template <typename TAsset, typename... TArgs>
  static AssetHandle CreateMemoryOnlyAsset(TArgs&&... args) {
    static_assert(
        std::is_base_of_v<Asset, TAsset>,
        "CreateMemoryOnlyAsset は Asset から派生した型に対してのみ機能します");

    Ref<TAsset> asset = Ref<TAsset>::Create(std::forward<TArgs>(args)...);
    asset->handle_ = AssetHandle();

    BASE_ENGINE(AssetManager)->AddMemoryOnlyAsset(asset);
    return asset->handle_;
  }

  template <typename TAsset, typename... TArgs>
  static AssetHandle CreateMemoryOnlyRendererAsset(TArgs&&... args) {
    static_assert(
        std::is_base_of_v<Asset, TAsset>,
        "CreateMemoryOnlyAsset は Asset から派生した型に対してのみ機能します");

    Ref<TAsset> asset = TAsset::Create(std::forward<TArgs>(args)...);
    asset->handle_ = AssetHandle();

    BASE_ENGINE(AssetManager)->AddMemoryOnlyAsset(asset);
    return asset->handle_;
  }

  // Editor
  static const AssetRegistry& GetAssetRegistry();
  static EditorAssetManager* GetEditorAssetManager();
};
}  // namespace base_engine
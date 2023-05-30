// @AssetRegistry.h
// @brief
// @author ICE
// @date 2023/04/05
//
// @details

#pragma once
#include "AssetMetadata.h"

namespace base_engine {
// TODO std::unordered_mapの利用をやめ独自のコンテナなどにするか最適化について考える
class AssetRegistry {
 public:
  AssetMetadata& operator[](const AssetHandle handle);
  AssetMetadata& Get(const AssetHandle handle);
  const AssetMetadata& Get(const AssetHandle handle) const;

  size_t Count() const { return asset_registry_.size(); }
  bool Contains(const AssetHandle handle) const;
  size_t Remove(const AssetHandle handle);
  void Clear();

  auto begin() { return asset_registry_.begin(); }
  auto end() { return asset_registry_.end(); }
  auto begin() const { return asset_registry_.cbegin(); }
  auto end() const { return asset_registry_.cend(); }

 private:
  std::unordered_map<AssetHandle, AssetMetadata> asset_registry_;
};
}  // namespace base_engine

#include "AssetRegistry.h"
using namespace base_engine;

static std::mutex s_asset_registry_mutex;

//TODO 各処理にログが出力されるようにする


AssetMetadata& AssetRegistry::operator[](const AssetHandle handle) {
  std::scoped_lock lock(s_asset_registry_mutex);
  return asset_registry_[handle];
}

const AssetMetadata& AssetRegistry::Get(const AssetHandle handle) const {
  std::scoped_lock lock(s_asset_registry_mutex);
  return asset_registry_.at(handle);
}

AssetMetadata& AssetRegistry::Get(const AssetHandle handle) {
  std::scoped_lock lock(s_asset_registry_mutex);

  return asset_registry_[handle];
}

bool AssetRegistry::Contains(const AssetHandle handle) const {
  std::scoped_lock lock(s_asset_registry_mutex);
  return asset_registry_.contains(handle);
}

size_t AssetRegistry::Remove(const AssetHandle handle) {
  std::scoped_lock lock(s_asset_registry_mutex);
  return asset_registry_.erase(handle);
}

void AssetRegistry::Clear() {
  std::scoped_lock lock(s_asset_registry_mutex);
  asset_registry_.clear();
}

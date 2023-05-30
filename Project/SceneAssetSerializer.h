// @SceneAssetSerializer.h
// @brief
// @author ICE
// @date 2023/05/15
//
// @details

#pragma once
#include "AssetSerializer.h"
namespace base_engine {
class SceneAssetSerializer : public base_engine::AssetSerializer {
 public:
  void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
  bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const;
  void GetRecognizedExtensions(std::list<std::string>* extensions) const;
  std::string GetAssetType(const std::filesystem::path& path) const;
};
}  // namespace base_engine

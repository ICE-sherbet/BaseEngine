// @AssetImporter.h
// @brief
// @author ICE
// @date 2023/04/04
//
// @details

#pragma once
#include <ranges>
#include <unordered_map>
#include <vector>

#include "AssetMetadata.h"
#include "AssetSerializer.h"
#include "AssetTypes.h"

namespace base_engine {
class AssetImporter {
 public:
  AssetImporter() = delete;

  static void Init();
  static void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset);
  static void Serialize(const Ref<Asset>& asset);
  static bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset);

  static AssetType GetAssetType(const std::filesystem::path& path) {
    for (const auto& [type, serializer] : serializers_) {
      if (std::string result = serializer->GetAssetType(path);
          !result.empty()) {
        return type;
      }
    }
    return AssetType::kNone;
  }

 private:
  static std::unordered_map<AssetType, std::unique_ptr<AssetSerializer>>
      serializers_;
};
}  // namespace base_engine

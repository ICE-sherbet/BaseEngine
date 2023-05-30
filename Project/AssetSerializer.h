// @AssetSerializer.h
// @brief
// @author ICE
// @date 2023/04/04
//
// @details

#pragma once
#include <cstdint>

#include "AssetMetadata.h"

namespace base_engine {
struct AssetSerializationInfo {
  uint64_t Offset = 0;
  uint64_t Size = 0;
};

__interface AssetSerializer {
 public:
  virtual void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset)
      const = 0;
  virtual bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset)
      const = 0;

  virtual void GetRecognizedExtensions(std::list<std::string>* extensions) const = 0;
  virtual std::string GetAssetType(const std::filesystem::path& path) const = 0;
};
}  // namespace base_engine

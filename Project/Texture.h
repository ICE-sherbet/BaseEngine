// @Texture.h
// @brief
// @author ICE
// @date 2023/04/04
//
// @details

#pragma once
#include "Asset.h"
#include "AssetSerializer.h"

namespace base_engine {
class Texture : public Asset {
 public:
  virtual AssetType GetAssetType() const override {
    return AssetType::kTexture;
  }
  virtual void* GetTexture() = 0;
};
class TextureUtility {
 public:
  static Ref<Texture> Create(const std::filesystem::path& path);
};

class TextureSerializer : public AssetSerializer {
 public:
  void Serialize(const AssetMetadata& metadata,
                 const Ref<Asset>& asset) const override {}
  bool TryLoadData(const AssetMetadata& metadata,
                   Ref<Asset>& asset) const override {
    asset = TextureUtility::Create(metadata.file_path);
    asset->handle_ = metadata.handle;
    return true;
  }

  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override {
    extensions->push_back(".png");
  }

  std::string GetAssetType(const std::filesystem::path& path) const override;
};
}  // namespace base_engine

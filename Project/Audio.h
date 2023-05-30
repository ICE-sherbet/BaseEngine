// @Audio.h
// @brief
// @author ICE
// @date 2023/04/18
//
// @details

#pragma once
#include "Asset.h"
#include "AssetSerializer.h"
#include "SoundBuffer.h"

namespace base_engine {
class Audio : public Asset {
 public:
  virtual AssetType GetAssetType() const override { return AssetType::kAudio; }

  virtual std::shared_ptr<ISoundBuffer> GetBuffer() const = 0;

 private:
};

class AudioUtilities {
 public:
  static Ref<Audio> Create(const std::filesystem::path& path);
};

class AudioSerializer : public AssetSerializer {
 public:
  void Serialize(const AssetMetadata& metadata,
                 const Ref<Asset>& asset) const override;

  bool TryLoadData(const AssetMetadata& metadata,
                   Ref<Asset>& asset) const override;

  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override;

  std::string GetAssetType(const std::filesystem::path& path) const override;
};
}  // namespace base_engine

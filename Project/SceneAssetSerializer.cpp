#include "SceneAssetSerializer.h"

#include "ObjectEntity.h"
#include "Scene.h"
#include "SceneSerializer.h"

namespace base_engine {
void SceneAssetSerializer::Serialize(const AssetMetadata& metadata,
                                     const Ref<Asset>& asset) const {
  SceneSerializer serializer(asset);
  serializer.Serialize(metadata.file_path);
}

bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata,
                                       Ref<Asset>& asset) const {
  asset = Ref<Scene>::Create("SceneAsset");
  asset->handle_ = metadata.handle;
  return true;
}

void SceneAssetSerializer::GetRecognizedExtensions(
    std::list<std::string>* extensions) const {
  extensions->push_back(".bscn");
}

std::string SceneAssetSerializer::GetAssetType(
    const std::filesystem::path& path) const {
  if (path.extension() == ".bscn") {
    return "Scene";
  }
  return "";
}
}  // namespace base_engine

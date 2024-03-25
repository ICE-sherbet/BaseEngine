#include "AssetImporter.h"

#include "AssetManager.h"
#include "Audio.h"
#include "MeshSerializer.h"
#include "Prefab.h"
#include "SceneAssetSerializer.h"
#include "Texture.h"

using namespace base_engine;
void AssetImporter::Init() {
  serializers_[AssetType::kTexture] = std::make_unique<TextureSerializer>();
  serializers_[AssetType::kAudio] = std::make_unique<AudioSerializer>();
  serializers_[AssetType::kScene] = std::make_unique<SceneAssetSerializer>();
  serializers_[AssetType::kPrefab] = std::make_unique<PrefabSerializer>();
  serializers_[AssetType::kMesh] = std::make_unique<MeshSerializer>();
  serializers_[AssetType::kMeshSource] = std::make_unique<MeshSourceSerializer>();
  serializers_[AssetType::kStaticMesh] = std::make_unique<StaticMeshSerializer>();
}

void AssetImporter::Serialize(const AssetMetadata& metadata,
                              const Ref<Asset>& asset) {
  if (!serializers_.contains(metadata.type)) {
    return;
  }
  serializers_[asset->GetAssetType()]->Serialize(metadata, asset);
}

void AssetImporter::Serialize(const Ref<Asset>& asset) {
  const AssetMetadata& metadata =
      AssetManager::GetMutableMetadata(asset->handle_);
  Serialize(metadata, asset);
}

bool AssetImporter::TryLoadData(const AssetMetadata& metadata,
                                Ref<Asset>& asset) {
  if (!serializers_.contains(metadata.type)) {
    return false;
  }
  return serializers_[metadata.type]->TryLoadData(metadata, asset);
}

std::unordered_map<AssetType, std::unique_ptr<AssetSerializer>>
    AssetImporter::serializers_;
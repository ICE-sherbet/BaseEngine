// @Prefab.h
// @brief
// @author ICE
// @date 2023/05/29
//
// @details

#pragma once
#include "Asset.h"
#include "AssetSerializer.h"
#include "ObjectEntity.h"
#include "Scene.h"

namespace base_engine {
class Prefab : public Asset {
 public:
  Prefab();
  static AssetType GetStaticType() { return AssetType::kPrefab; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

  void Create(ObjectEntity entity, bool serialize = true);

 private:
  ObjectEntity CreatePrefabFromEntity(ObjectEntity entity);

  Ref<Scene> scene_;
  ObjectEntity entity_;
  friend class PrefabSerializer;
  friend class Scene;
};

class PrefabSerializer : public AssetSerializer {
 public:
  void Serialize(const AssetMetadata& metadata,
                 const Ref<Asset>& asset) const override;

  bool TryLoadData(const AssetMetadata& metadata,
                   Ref<Asset>& asset) const override;

  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override;

  std::string GetAssetType(const std::filesystem::path& path) const override;
private:
  std::string SerializeToYAML(Ref<Prefab> prefab) const;
  bool DeserializeFromYAML(const std::string& yaml_string,
                           Ref<Prefab> prefab) const;
};
}  // namespace base_engine

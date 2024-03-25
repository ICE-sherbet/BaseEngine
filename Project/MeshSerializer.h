// @MeshSerializer.h
// @brief
// @author ICE
// @date 2024/03/10
//
// @details

#pragma once
#include "AssetSerializer.h"
#include "Mesh.h"
#include "StaticMesh.h"

namespace base_engine {

class MeshSourceSerializer : public AssetSerializer {
 public:
  virtual void Serialize(const AssetMetadata& metadata,
                         const Ref<Asset>& asset) const override {}
  virtual bool TryLoadData(const AssetMetadata& metadata,
                           Ref<Asset>& asset) const override;
  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override {
    extensions->push_back(".meshs");
  }

  [[nodiscard]] std::string GetAssetType(
      const std::filesystem::path& path) const override {
    if (path.extension() == ".meshs") {
      return "MeshSource";
    }
    return "";
  };
};

class MeshSerializer : public AssetSerializer {
 public:
  virtual void Serialize(const AssetMetadata& metadata,
                         const Ref<Asset>& asset) const override;
  virtual bool TryLoadData(const AssetMetadata& metadata,
                           Ref<Asset>& asset) const override;

 private:
  std::string SerializeToYAML(Ref<Mesh> mesh) const;
  bool DeserializeFromYAML(const std::string& yaml_string,
                           Ref<Mesh>& target_mesh) const;

 public:
  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override {
    extensions->push_back(".mesh");
  }

  [[nodiscard]] std::string GetAssetType(
      const std::filesystem::path& path) const override {
    return "Mesh";
  }
};

class StaticMeshSerializer : public AssetSerializer {
 public:
  virtual void Serialize(const AssetMetadata& metadata,
                         const Ref<Asset>& asset) const override;
  virtual bool TryLoadData(const AssetMetadata& metadata,
                           Ref<Asset>& asset) const override;

 private:
  [[nodiscard]] std::string SerializeToYAML(Ref<StaticMesh> staticMesh) const;
  bool DeserializeFromYAML(const std::string& yaml_string,
                           Ref<StaticMesh>& target_static_mesh) const;

 public:
  void GetRecognizedExtensions(
      std::list<std::string>* extensions) const override {
    extensions->push_back(".smesh");
  }

  [[nodiscard]] std::string GetAssetType(
      const std::filesystem::path& path) const override {
    return "StaticMesh";
  }
};
}  // namespace base_engine

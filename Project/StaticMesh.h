// @StaticMesh.h
// @brief
// @author ICE
// @date 2024/03/11
//
// @details

#pragma once
#include "Asset.h"
#include "MaterialAsset.h"
#include "MeshSource.h"

namespace base_engine {
class StaticMesh : public Asset {
 public:
  explicit StaticMesh(Ref<MeshSource> mesh_source);
  explicit StaticMesh(const Ref<StaticMesh>& other);
  StaticMesh(Ref<MeshSource> mesh_source,
             const std::vector<uint32_t>& submeshes);
  virtual ~StaticMesh();

  std::vector<uint32_t>& GetSubmeshes() { return submeshes_; }
  const std::vector<uint32_t>& GetSubmeshes() const { return submeshes_; }

  void SetSubmeshes(const std::vector<uint32_t>& submeshes);

  Ref<MeshSource> GetMeshSource() { return mesh_source_; }
  Ref<MeshSource> GetMeshSource() const { return mesh_source_; }
  void SetMeshAsset(const Ref<MeshSource>& mesh_asset) { mesh_source_ = mesh_asset; }

  Ref<MaterialTable> GetMaterials() const { return materials_; }

  static AssetType GetStaticType() { return AssetType::kStaticMesh; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

 private:
  Ref<MeshSource> mesh_source_;
  std::vector<uint32_t> submeshes_;
  Ref<MaterialTable> materials_;
};
}  // namespace base_engine

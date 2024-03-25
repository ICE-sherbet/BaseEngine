// @kMesh.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include "Asset.h"
#include "MaterialAsset.h"
#include "MeshSource.h"

namespace base_engine {
class Mesh : public Asset {
 public:
  explicit Mesh(Ref<MeshSource> mesh_source);
  Mesh(Ref<MeshSource> mesh_source, const std::vector<uint32_t>& submeshes);
  explicit Mesh(const Ref<Mesh>& other);
  ~Mesh() override;

  bool HasSkeleton() { return mesh_source_ && mesh_source_->HasSkeleton(); }

  std::vector<uint32_t>& GetSubmeshes() { return submeshes_; }
  const std::vector<uint32_t>& GetSubmeshes() const { return submeshes_; }

  void SetSubmeshes(const std::vector<uint32_t>& submeshes);

  Ref<MeshSource> GetMeshSource() { return mesh_source_; }
  Ref<MeshSource> GetMeshSource() const { return mesh_source_; }
  void SetMeshAsset(const Ref<MeshSource>& mesh_source) {
    mesh_source_ = mesh_source;
  }

  Ref<MaterialTable> GetMaterials() const { return materials_; }

  static AssetType GetStaticType() { return AssetType::kMesh; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

 private:
  Ref<MeshSource> mesh_source_;
  std::vector<uint32_t> submeshes_;

  Ref<MaterialTable> materials_;
};

}  // namespace base_engine

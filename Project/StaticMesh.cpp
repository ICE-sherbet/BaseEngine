#include "StaticMesh.h"

#include "AssetManager.h"

namespace base_engine {
StaticMesh::StaticMesh(Ref<MeshSource> mesh_source) : mesh_source_(mesh_source) {
  handle_ = {};

  SetSubmeshes({});

  const auto& mesh_materials = mesh_source->GetMaterials();
  uint32_t num_materials = mesh_materials.size();
  materials_ = Ref<MaterialTable>::Create(num_materials);
  for (uint32_t i = 0; i < num_materials; i++)
    materials_->SetMaterial(
        i,
        AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(mesh_materials[i]));
}

StaticMesh::StaticMesh(Ref<MeshSource> mesh_source,
                       const std::vector<uint32_t>& submeshes)
    : mesh_source_(mesh_source) {
  handle_ = {};

  SetSubmeshes(submeshes);

  const auto& mesh_materials = mesh_source->GetMaterials();
  auto num_materials = mesh_materials.size();
  materials_ = Ref<MaterialTable>::Create(num_materials);
  for (uint32_t i = 0; i < num_materials; i++)
    materials_->SetMaterial(
        i,
        AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(mesh_materials[i]));
}

StaticMesh::StaticMesh(const Ref<StaticMesh>& other)
    : mesh_source_(other->mesh_source_), materials_(other->materials_) {
  SetSubmeshes(other->submeshes_);
}

StaticMesh::~StaticMesh() = default;

void StaticMesh::SetSubmeshes(const std::vector<uint32_t>& submeshes) {
  if (!submeshes.empty()) {
    submeshes_ = submeshes;
  } else {
    const auto& submeshes = mesh_source_->GetSubmeshes();
    submeshes_.resize(submeshes.size());
    for (uint32_t i = 0; i < submeshes.size(); i++) submeshes_[i] = i;
  }
}
}  // namespace base_engine

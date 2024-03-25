#include "Mesh.h"

#include "AssetManager.h"

namespace base_engine {
Mesh::Mesh(Ref<MeshSource> mesh_source) : mesh_source_(mesh_source) {
  handle_ = {};

  SetSubmeshes({});

  const auto& mesh_materials = mesh_source->GetMaterials();
  materials_ = Ref<MaterialTable>::Create((uint32_t)mesh_materials.size());
  for (size_t i = 0; i < mesh_materials.size(); i++)
    materials_->SetMaterial(
        i,
        AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(mesh_materials[i]));
}

Mesh::Mesh(Ref<MeshSource> mesh_source, const std::vector<uint32_t>& submeshes)
    : mesh_source_(mesh_source) {
  handle_ = {};

  SetSubmeshes(submeshes);

  const auto& meshMaterials = mesh_source->GetMaterials();
  materials_ = Ref<MaterialTable>::Create(meshMaterials.size());
  for (size_t i = 0; i < meshMaterials.size(); i++)
    materials_->SetMaterial(
        i,
        AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(meshMaterials[i]));
}

Mesh::Mesh(const Ref<Mesh>& other)
    : mesh_source_(other->mesh_source_), materials_(other->materials_) {
  SetSubmeshes(other->submeshes_);
}

Mesh::~Mesh() = default;

void Mesh::SetSubmeshes(const std::vector<uint32_t>& submeshes) {
  if (!submeshes.empty()) {
    submeshes_ = submeshes;
  } else {
    const auto& submeshes = mesh_source_->GetSubmeshes();
    submeshes_.resize(submeshes.size());
    for (uint32_t i = 0; i < submeshes.size(); i++) submeshes_[i] = i;
  }
}
}  // namespace base_engine

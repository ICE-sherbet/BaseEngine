#include "MeshSerializer.h"

#include <fstream>

#include "AssetManager.h"
#include "MeshImporter.h"
#include "yaml-cpp/emitter.h"
#include "yaml-cpp/yaml.h"

namespace base_engine {

YAML::Emitter& operator<<(YAML::Emitter& out,
                          const std::vector<uint32_t>& value) {
  out << YAML::Flow;
  out << YAML::BeginSeq;
  for (const uint32_t element : value) out << element;
  out << YAML::EndSeq;
  return out;
}

bool MeshSourceSerializer::TryLoadData(const AssetMetadata& metadata,
                                       Ref<Asset>& asset) const {
  MeshImporter importer(metadata.file_path);
  Ref<MeshSource> mesh_source = importer.ImportToMeshSource();
  if (!mesh_source) return false;

  asset = mesh_source;
  asset->handle_ = metadata.handle;
  return true;
}

void MeshSerializer::Serialize(const AssetMetadata& metadata,
                               const Ref<Asset>& asset) const {
  const Ref<Mesh> mesh = asset.As<Mesh>();

  const std::string yaml_string = SerializeToYAML(mesh);

  std::ofstream fout(metadata.file_path);

  if (!fout.is_open()) {
    return;
  }

  fout << yaml_string;
  fout.flush();
  fout.close();
}

bool MeshSerializer::TryLoadData(const AssetMetadata& metadata,
                                 Ref<Asset>& asset) const {
  const auto filepath = metadata.file_path;
  const std::ifstream stream(filepath);
  BE_CORE_ASSERT(stream);
  std::stringstream str_stream;
  str_stream << stream.rdbuf();

  Ref<Mesh> mesh;
  if (const bool success = DeserializeFromYAML(str_stream.str(), mesh);
      !success)
    return false;

  mesh->handle_ = metadata.handle;
  asset = mesh;
  return true;
}

std::string MeshSerializer::SerializeToYAML(Ref<Mesh> mesh) const {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Mesh";
  {
    out << YAML::BeginMap;
    out << YAML::Key << "MeshSource";
    out << YAML::Value << mesh->GetMeshSource()->handle_;
    out << YAML::Key << "SubmeshIndices";
    out << YAML::Flow;
    if (mesh->GetSubmeshes().size() ==
        mesh->GetMeshSource()->GetSubmeshes().size())
      out << YAML::Value << std::vector<uint32_t>();
    else
      out << YAML::Value << mesh->GetSubmeshes();
    out << YAML::EndMap;
  }
  out << YAML::EndMap;

  return out.c_str();
}

bool MeshSerializer::DeserializeFromYAML(const std::string& yaml_string,
                                         Ref<Mesh>& target_mesh) const {
  YAML::Node data = YAML::Load(yaml_string);
  if (!data["Mesh"]) return false;

  YAML::Node root_node = data["Mesh"];
  if (!root_node["MeshAsset"] && !root_node["MeshSource"]) return false;

  AssetHandle mesh_source_handle = 0;
  if (root_node["MeshAsset"])
    mesh_source_handle = root_node["MeshAsset"].as<uint64_t>();
  else
    mesh_source_handle = root_node["MeshSource"].as<uint64_t>();

  Ref<MeshSource> mesh_source =
      AssetManager::GetAsset<MeshSource>(mesh_source_handle);
  if (!mesh_source) return false;

  auto submesh_indices =
      root_node["SubmeshIndices"].as<std::vector<uint32_t>>();
  target_mesh = Ref<Mesh>::Create(mesh_source, submesh_indices);
  return true;
}

void StaticMeshSerializer::Serialize(const AssetMetadata& metadata,
                                     const Ref<Asset>& asset) const {
  const Ref<StaticMesh> static_mesh = asset.As<StaticMesh>();

  const std::string yaml_string = SerializeToYAML(static_mesh);

  const auto serialize_path = metadata.file_path;
  std::ofstream fout(serialize_path);
  BE_CORE_ASSERT(fout.good());
  if (fout.good()) fout << yaml_string;
}

bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata,
                                       Ref<Asset>& asset) const {
  const auto filepath = metadata.file_path;
  const std::ifstream stream(filepath);
  BE_CORE_ASSERT(stream);
  std::stringstream str_stream;
  str_stream << stream.rdbuf();

  Ref<StaticMesh> static_mesh;
  if (const bool success = DeserializeFromYAML(str_stream.str(), static_mesh);
      !success)
    return false;

  static_mesh->handle_ = metadata.handle;
  asset = static_mesh;
  return true;
}

std::string StaticMeshSerializer::SerializeToYAML(
    Ref<StaticMesh> staticMesh) const {
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "Mesh";
  {
    out << YAML::BeginMap;
    out << YAML::Key << "MeshSource";
    out << YAML::Value << staticMesh->GetMeshSource()->handle_;
    out << YAML::Key << "SubmeshIndices";
    out << YAML::Flow;
    if (staticMesh->GetSubmeshes().size() ==
        staticMesh->GetMeshSource()->GetSubmeshes().size())
      out << YAML::Value << std::vector<uint32_t>();
    else
      out << YAML::Value << staticMesh->GetSubmeshes();
    out << YAML::EndMap;
  }
  out << YAML::EndMap;

  return out.c_str();
}

bool StaticMeshSerializer::DeserializeFromYAML(
    const std::string& yaml_string, Ref<StaticMesh>& target_static_mesh) const {
  YAML::Node data = YAML::Load(yaml_string);
  if (!data["Mesh"]) return false;

  YAML::Node root_node = data["Mesh"];
  if (!root_node["MeshSource"] && !root_node["MeshAsset"]) return false;

  const AssetHandle mesh_source_handle = root_node["MeshSource"].as<uint64_t>();
  Ref<MeshSource> mesh_source =
      AssetManager::GetAsset<MeshSource>(mesh_source_handle);
  if (!mesh_source) return false;

  auto submesh_indices = root_node["SubmeshIndices"].as<std::vector<uint32_t>>();
  target_static_mesh = Ref<StaticMesh>::Create(mesh_source, submesh_indices);

  return true;
}
}  // namespace base_engine

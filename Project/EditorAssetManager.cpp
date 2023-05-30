#include "EditorAssetManager.h"

#include <cstring>
#include <fstream>
#include <ranges>
#include <string>

#include <yaml-cpp/yaml.h>
using namespace base_engine;
static AssetMetadata kNullMetadata;

EditorAssetManager::EditorAssetManager() {
}

void EditorAssetManager::Initialize()
{
  AssetImporter::Init();
  LoadAssetRegistry();
  ReloadAssetFiles();
  WriteRegistryToFile();
}

EditorAssetManager::~EditorAssetManager() { WriteRegistryToFile(); }

AssetType EditorAssetManager::GetAssetType(const AssetHandle asset_handle)
{
	return asset_registry_.Get(asset_handle).type;
}

void EditorAssetManager::AddMemoryOnlyAsset(Ref<Asset> asset)
{
	AssetMetadata metadata;
	metadata.handle = asset->handle_;
	metadata.is_data_loaded = true;
	metadata.type = asset->GetAssetType();
	metadata.is_memory_asset = true;
	asset_registry_[metadata.handle] = metadata;

	memory_assets_[asset->handle_] = asset;
}

bool EditorAssetManager::ReloadData(AssetHandle asset_handle)
{ return false; }

bool EditorAssetManager::IsAssetHandleValid(const AssetHandle asset_handle)
{
	return IsMemoryAsset(asset_handle) || GetMetadata(asset_handle).IsValid();
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
{ return false; }

std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type)
{
	return {};
}

const std::unordered_map<AssetHandle, Ref<Asset>>& EditorAssetManager::GetLoadedAssets()
{
	return {};
}

const std::unordered_map<AssetHandle, Ref<Asset>>& EditorAssetManager::GetMemoryOnlyAssets()
{
	return {};
}

Ref<Asset> EditorAssetManager::GetAsset(const std::filesystem::path file_path)
{
	return GetAsset(GetMetadata(file_path).handle);
}

const AssetRegistry& EditorAssetManager::GetAssetRegistry() const
{ return asset_registry_; }

Ref<Asset> EditorAssetManager::GetAsset(const AssetHandle asset_handle) {
  if (IsMemoryAsset(asset_handle)) return memory_assets_[asset_handle];

  auto& metadata = GetMetadataInternal(asset_handle);
  if (!metadata.IsValid()) return nullptr;
  Ref<Asset> asset = nullptr;
  if (!metadata.is_data_loaded) {
    metadata.is_data_loaded = AssetImporter::TryLoadData(metadata, asset);
    if (!metadata.is_data_loaded) return nullptr;

    loaded_assets_[asset_handle] = asset;
  } else {
    asset = loaded_assets_[asset_handle];
  }

  return asset;
}

bool EditorAssetManager::IsMemoryAsset(AssetHandle handle) {
  return memory_assets_.contains(handle);
}

const AssetMetadata& EditorAssetManager::GetMetadata(const AssetHandle handle) {
  if (asset_registry_.Contains(handle)) return asset_registry_[handle];

  return kNullMetadata;
}

const AssetMetadata& EditorAssetManager::GetMetadata(
    const std::filesystem::path& filepath) {
  for (auto& metadata : asset_registry_ | std::views::values) {
    if (metadata.file_path == filepath) return metadata;
  }

  return kNullMetadata;
}

// TODO Godotなどを参考にしてImport周りの実装の拡張性をあげる
AssetHandle EditorAssetManager::ImportAsset(
    const std::filesystem::path& filepath) {
  if (auto& metadata = GetMetadata(filepath); metadata.IsValid())
    return metadata.handle;

  const AssetType type = GetAssetTypeFromPath(filepath);
  if (type == AssetType::kNone) return 0;

  AssetMetadata metadata;
  metadata.handle = AssetHandle();
  metadata.file_path = filepath;
  metadata.type = type;
  asset_registry_[metadata.handle] = metadata;

  return metadata.handle;
}

AssetType EditorAssetManager::GetAssetTypeFromPath(
    const std::filesystem::path& path) {
  return AssetImporter::GetAssetType(path);
}

AssetMetadata& EditorAssetManager::GetMutableMetadata(AssetHandle handle) {
  if (asset_registry_.Contains(handle)) return asset_registry_[handle];

  return kNullMetadata;
}
void EditorAssetManager::LoadAssetRegistry()
{
  BE_CORE_INFO("[AssetManager] Loading Asset Registry");

  const auto& asset_registry_path = "assets.be";
  if (!std::filesystem::exists(asset_registry_path)) return;

  std::ifstream stream(asset_registry_path);
  std::stringstream str_stream;
  str_stream << stream.rdbuf();

  YAML::Node data = YAML::Load(str_stream.str());
  YAML::Node handles = data["Assets"];
  if (!handles) {
    return;
  }

  for (auto entry : handles) {
	  auto filepath = entry["FilePath"].as<std::string>();

    AssetMetadata metadata;
    metadata.handle = entry["Handle"].as<uint64_t>();
    metadata.file_path = filepath;
    metadata.type =
        AssetUtilities::AssetTypeFromString(entry["Type"].as<std::string>());

    if (metadata.type == AssetType::kNone) continue;

    if (metadata.type != GetAssetTypeFromPath(filepath)) {

      metadata.type = GetAssetTypeFromPath(filepath);
    }

    if (!std::filesystem::exists(metadata.file_path)) {

      std::string mostLikelyCandidate;
      uint32_t bestScore = 0;
      for (auto& pathEntry : std::filesystem::recursive_directory_iterator(
               std::filesystem::current_path())) {
        const std::filesystem::path& path = pathEntry.path();

        if (path.filename() != metadata.file_path.filename()) continue;

      	uint32_t score = 0;
        for (const auto& part : path) {
          if (filepath.find(part.string()) != std::string::npos) score++;
        }

        if (bestScore > 0 && score == bestScore) {
          // TODO: How do we handle this?
          // Probably prompt the user at this point?
        }

        if (score <= bestScore) continue;

        bestScore = score;
        mostLikelyCandidate = path.string();
      }

      if (mostLikelyCandidate.empty() && bestScore == 0) {
        continue;
      }
      
      metadata.file_path = std::filesystem::path{mostLikelyCandidate}.make_preferred();

    }

    if (metadata.handle == 0) {
      continue;
    }

    asset_registry_[metadata.handle] = metadata;
  }
  
}

void EditorAssetManager::ReloadAssetFiles()
{
  for (const std::filesystem::directory_entry& x :
       std::filesystem::recursive_directory_iterator(".")) {
    const auto str = proximate(x.path());
    ImportAsset(str);

  }
}

void EditorAssetManager::WriteRegistryToFile() {
  struct AssetRegistryEntry {
    std::string file_path;
    AssetType type;
  };

  using AssetRegistryMap = std::map<UUID, AssetRegistryEntry>;
  AssetRegistryMap sorted_map;
  for (auto& metadata : asset_registry_ | std::views::values) {
    if (!std::filesystem::exists(metadata.file_path)) continue;

    if (metadata.is_memory_asset) continue;

    const std::string path_to_serialize =
        metadata.file_path.make_preferred().string();
    sorted_map[metadata.handle] = {path_to_serialize, metadata.type};
  }

  YAML::Emitter out;
  out << YAML::BeginMap;

  out << YAML::Key << "Assets" << YAML::BeginSeq;
  for (auto& [handle, entry] : sorted_map) {
    out << YAML::BeginMap;
    out << YAML::Key << "Handle" << YAML::Value << handle;
    out << YAML::Key << "FilePath" << YAML::Value << entry.file_path;
    out << YAML::Key << "Type" << YAML::Value
        << AssetUtilities::AssetTypeToString(entry.type).data();
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;

  const std::string& asset_registry_path = "assets.be";
  std::ofstream fout(asset_registry_path);
  fout << out.c_str();
}

AssetMetadata& EditorAssetManager::GetMetadataInternal(
    const AssetHandle handle) {
  if (asset_registry_.Contains(handle)) return asset_registry_[handle];
  return kNullMetadata;
}

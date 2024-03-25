// @AssetTypes.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <cstdint>

#include "Assert.h"

namespace base_engine {
enum class AssetFlag : uint16_t {
  kNone = 0,
  kMissing = std::rotl(1u, 0),
  kInvalid = std::rotl(1u, 1)
};

enum class AssetType : uint16_t {
  kNone = 0,
  kScene,
  kTexture,
  kScript,
  kAudio,
  kPrefab,
  kMeshSource,
  kSkeleton,
  kAnimation,
  kMesh,
  kStaticMesh,
  kMaterial,
  kCount
};

class AssetUtilities {
  struct AssetNVP {
    AssetType type;
    std::string_view name;

    constexpr AssetNVP(const AssetType type, const std::string_view& name)
        : type(type), name(name) {}
  };
  constexpr static auto GetAssetsNVP() {
    return std::array<AssetNVP, static_cast<int>(AssetType::kCount)>{
        AssetNVP{AssetType::kNone, "None"},
        AssetNVP{AssetType::kScene, "Scene"},
        AssetNVP{AssetType::kTexture, "Texture"},
        AssetNVP{AssetType::kScript, "Script"},
        AssetNVP{AssetType::kAudio, "Audio"},
        AssetNVP{AssetType::kPrefab, "Prefab"},
        AssetNVP{AssetType::kMeshSource, "MeshSource"},
        AssetNVP{AssetType::kStaticMesh, "StaticMesh"},
        AssetNVP{AssetType::kSkeleton, "Skeleton"},
        AssetNVP{AssetType::kAnimation, "Animation"},
        AssetNVP{AssetType::kMesh, "Mesh"},
        AssetNVP{AssetType::kMaterial, "kMaterial"},
    };
  }

 public:
  AssetUtilities() = delete;

  constexpr static AssetType AssetTypeFromString(std::string_view asset_type) {
    const auto& NVPs = GetAssetsNVP();
    for (const auto& pair : NVPs) {
      if (pair.name == asset_type) return pair.type;
    }
    BE_CORE_ASSERT(false, "Unknown Asset Type");
    return AssetType::kNone;
  }
  constexpr static std::string_view AssetTypeToString(AssetType type) {
    switch (type) {
      case AssetType::kScene:
        break;
      case AssetType::kTexture:
        break;
      case AssetType::kScript:
        break;
      case AssetType::kAudio:
        break;
      case AssetType::kCount:
        break;
      case AssetType::kPrefab:
        break;
      case AssetType::kMeshSource:
        break;
      case AssetType::kSkeleton:
        break;
      case AssetType::kAnimation:
        break;
      case AssetType::kMesh:
        break;
      case AssetType::kMaterial:
        break;
      case AssetType::kNone:
      default:
        BE_CORE_ASSERT(false, "Assetが存在しません。")
        return "None";
    }
    return GetAssetsNVP()[static_cast<int>(type)].name;
  }
};
}  // namespace base_engine

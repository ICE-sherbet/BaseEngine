// @MaterialAsset.h
// @brief
// @author ICE
// @date 2024/02/27
//
// @details

#pragma once
#include <cstdint>
#include <glm/vec3.hpp>

#include "Asset.h"
#include "Material.h"

namespace base_engine {
class MaterialAsset : public Asset {
 public:
  MaterialAsset(bool transparent = false);
  MaterialAsset(Ref<Material> material);
  ~MaterialAsset();

  glm::vec3& GetAlbedoColor();
  void SetAlbedoColor(const glm::vec3& color);

  float& GetMetalness();
  void SetMetalness(float value);

  float& GetRoughness();
  void SetRoughness(float value);

  float& GetEmission();
  void SetEmission(float value);

  Ref<RendererTexture2D> GetAlbedoMap();
  void SetAlbedoMap(Ref<RendererTexture2D> texture);
  void ClearAlbedoMap();

  Ref<RendererTexture2D> GetNormalMap();
  void SetNormalMap(Ref<RendererTexture2D> texture);
  bool IsUsingNormalMap();
  void SetUseNormalMap(bool value);
  void ClearNormalMap();

  Ref<RendererTexture2D> GetMetalnessMap();
  void SetMetalnessMap(Ref<RendererTexture2D> texture);
  void ClearMetalnessMap();

  Ref<RendererTexture2D> GetRoughnessMap();
  void SetRoughnessMap(Ref<RendererTexture2D> texture);
  void ClearRoughnessMap();

  float& GetTransparency();
  void SetTransparency(float transparency);

  bool IsShadowCasting() const {
    return !material_->GetFlag(MaterialFlag::DisableShadowCasting);
  }
  void SetShadowCasting(bool castsShadows) {
    return material_->SetFlag(MaterialFlag::DisableShadowCasting,
                               !castsShadows);
  }

  static AssetType GetStaticType() { return AssetType::kMaterial; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

  Ref<Material> GetMaterial() const { return material_; }
  void SetMaterial(const Ref<Material>& material) { material_ = material; }

  bool IsTransparent() const { return transparent_; }

 private:
  void SetDefaults();

 private:
  Ref<Material> material_;
  bool transparent_ = false;

  friend class MaterialEditor;
};

class MaterialTable : public RefCounted {
 public:
  MaterialTable(uint32_t materialCount = 1);
  MaterialTable(Ref<MaterialTable> other);
  ~MaterialTable() = default;

  bool HasMaterial(uint32_t materialIndex) const {
    return materials_.contains(materialIndex);
  }
  void SetMaterial(uint32_t index, AssetHandle material);
  void ClearMaterial(uint32_t index);

  AssetHandle GetMaterial(uint32_t materialIndex) const {
    BE_CORE_ASSERT(HasMaterial(materialIndex));
    return materials_.at(materialIndex);
  }
  std::map<uint32_t, AssetHandle>& GetMaterials() { return materials_; }
  const std::map<uint32_t, AssetHandle>& GetMaterials() const {
    return materials_;
  }

  uint32_t GetMaterialCount() const { return material_count_; }
  void SetMaterialCount(uint32_t materialCount) {
    material_count_ = materialCount;
  }

  void Clear();

 private:
  std::map<uint32_t, AssetHandle> materials_;
  uint32_t material_count_;
};
}  // namespace base_engine

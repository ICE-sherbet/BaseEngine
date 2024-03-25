#include "MaterialAsset.h"

#include "RendererApi.h"

namespace base_engine {
static const std::string s_AlbedoColorUniform =
    "u_MaterialUniforms.AlbedoColor";
static const std::string s_UseNormalMapUniform =
    "u_MaterialUniforms.UseNormalMap";
static const std::string s_MetalnessUniform = "u_MaterialUniforms.Metalness";
static const std::string s_RoughnessUniform = "u_MaterialUniforms.Roughness";
static const std::string s_EmissionUniform = "u_MaterialUniforms.Emission";
static const std::string s_TransparencyUniform =
    "u_MaterialUniforms.Transparency";

static const std::string s_AlbedoMapUniform = "u_AlbedoTexture";
static const std::string s_NormalMapUniform = "u_NormalTexture";
static const std::string s_MetalnessMapUniform = "u_MetalnessTexture";
static const std::string s_RoughnessMapUniform = "u_RoughnessTexture";

MaterialAsset::MaterialAsset(bool transparent) : transparent_(transparent) {
  handle_ = {};

  if (transparent)
    material_ = Material::Create(
        Renderer::GetShaderLibrary()->Get("HazelPBR_Transparent"));
  else
    material_ =
        Material::Create(Renderer::GetShaderLibrary()->Get("HazelPBR_Static"));

  SetDefaults();
}

MaterialAsset::MaterialAsset(Ref<Material> material) {
  handle_ = {};
  material_ = Material::Copy(material);
}

MaterialAsset::~MaterialAsset() {}

glm::vec3& MaterialAsset::GetAlbedoColor() {
  return material_->GetVector3(s_AlbedoColorUniform);
}

void MaterialAsset::SetAlbedoColor(const glm::vec3& color) {
  material_->Set(s_AlbedoColorUniform, color);
}

float& MaterialAsset::GetMetalness() {
  return material_->GetFloat(s_MetalnessUniform);
}

void MaterialAsset::SetMetalness(float value) {
  material_->Set(s_MetalnessUniform, value);
}

float& MaterialAsset::GetRoughness() {
  return material_->GetFloat(s_RoughnessUniform);
}

void MaterialAsset::SetRoughness(float value) {
  material_->Set(s_RoughnessUniform, value);
}

float& MaterialAsset::GetEmission() {
  return material_->GetFloat(s_EmissionUniform);
}

void MaterialAsset::SetEmission(float value) {
  material_->Set(s_EmissionUniform, value);
}

Ref<RendererTexture2D> MaterialAsset::GetAlbedoMap() {
  return material_->TryGetTexture2D(s_AlbedoMapUniform);
}

void MaterialAsset::SetAlbedoMap(Ref<RendererTexture2D> texture) {
  material_->Set(s_AlbedoMapUniform, texture);
}

void MaterialAsset::ClearAlbedoMap() {
  material_->Set(s_AlbedoMapUniform, Renderer::GetWhiteTexture());
}

Ref<RendererTexture2D> MaterialAsset::GetNormalMap() {
  return material_->TryGetTexture2D(s_NormalMapUniform);
}

void MaterialAsset::SetNormalMap(Ref<RendererTexture2D> texture) {
  material_->Set(s_NormalMapUniform, texture);
}

bool MaterialAsset::IsUsingNormalMap() {
  return material_->GetBool(s_UseNormalMapUniform);
}

void MaterialAsset::SetUseNormalMap(bool value) {
  material_->Set(s_UseNormalMapUniform, value);
}

void MaterialAsset::ClearNormalMap() {
  material_->Set(s_NormalMapUniform, Renderer::GetWhiteTexture());
}

Ref<RendererTexture2D> MaterialAsset::GetMetalnessMap() {
  return material_->TryGetTexture2D(s_MetalnessMapUniform);
}

void MaterialAsset::SetMetalnessMap(Ref<RendererTexture2D> texture) {
  material_->Set(s_MetalnessMapUniform, texture);
}

void MaterialAsset::ClearMetalnessMap() {
  material_->Set(s_MetalnessMapUniform, Renderer::GetWhiteTexture());
}

Ref<RendererTexture2D> MaterialAsset::GetRoughnessMap() {
  return material_->TryGetTexture2D(s_RoughnessMapUniform);
}

void MaterialAsset::SetRoughnessMap(Ref<RendererTexture2D> texture) {
  material_->Set(s_RoughnessMapUniform, texture);
}

void MaterialAsset::ClearRoughnessMap() {
  material_->Set(s_RoughnessMapUniform, Renderer::GetWhiteTexture());
}

float& MaterialAsset::GetTransparency() {
  return material_->GetFloat(s_TransparencyUniform);
}

void MaterialAsset::SetTransparency(float transparency) {
  material_->Set(s_TransparencyUniform, transparency);
}

void MaterialAsset::SetDefaults() {
  if (transparent_) {
    // Set defaults
    SetAlbedoColor(glm::vec3(0.8f));

    // Maps
    SetAlbedoMap(Renderer::GetWhiteTexture());
  } else {
    // Set defaults
    SetAlbedoColor(glm::vec3(0.8f));
    SetEmission(0.0f);
    SetUseNormalMap(false);
    SetMetalness(0.0f);
    SetRoughness(0.4f);

    // Maps
    SetAlbedoMap(Renderer::GetWhiteTexture());
    SetNormalMap(Renderer::GetWhiteTexture());
    SetMetalnessMap(Renderer::GetWhiteTexture());
    SetRoughnessMap(Renderer::GetWhiteTexture());
  }
}

MaterialTable::MaterialTable(uint32_t materialCount)
    : material_count_(materialCount) {}

MaterialTable::MaterialTable(Ref<MaterialTable> other)
    : material_count_(other->material_count_) {
  const auto& meshMaterials = other->GetMaterials();
  for (auto [index, materialAsset] : meshMaterials)
    // SetMaterial(index,
    // Ref<MaterialAsset>::Create(materialAsset->GetMaterial()));
    SetMaterial(index, materialAsset);
}

void MaterialTable::SetMaterial(uint32_t index, AssetHandle material) {
  materials_[index] = material;
  if (index >= material_count_) material_count_ = index + 1;
}

void MaterialTable::ClearMaterial(uint32_t index) {
  BE_CORE_ASSERT(HasMaterial(index));
  materials_.erase(index);
  if (index >= material_count_) material_count_ = index + 1;
}

void MaterialTable::Clear() { materials_.clear(); }
}  // namespace base_engine

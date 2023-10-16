#include "SpriteRendererComponent.h"

namespace base_engine::component {
void SpriteRendererComponent::_Bind() {
  ComponentDB::BindMethod("SetColor", &SpriteRendererComponent::SetColor);
  ComponentDB::BindMethod("GetColor", &SpriteRendererComponent::Color);
  ADD_PROPERTY(PropertyInfo(VariantType::kCOLOR, "color"), "SetColor",
               "GetColor");

  ComponentDB::BindMethod("SetTexture", &SpriteRendererComponent::SetTexture);
  ComponentDB::BindMethod("GetTexture", &SpriteRendererComponent::Texture);
  ADD_PROPERTY(PropertyInfo(VariantType::kAssetHandle, "texture",
                            PropertyHint::kAsset, "Texture"),
               "SetTexture", "GetTexture");
 
  ComponentDB::BindMethod("SetUvStart", &SpriteRendererComponent::SetUvStart);
  ComponentDB::BindMethod("GetUvStart", &SpriteRendererComponent::UvStart);
  ADD_PROPERTY(PropertyInfo(VariantType::kVECTOR2F, "uv_start"), "SetUvStart",
               "GetUvStart");

  ComponentDB::BindMethod("SetUvEnd", &SpriteRendererComponent::SetUvEnd);
  ComponentDB::BindMethod("GetUvEnd", &SpriteRendererComponent::UvEnd);
  ADD_PROPERTY(PropertyInfo(VariantType::kVECTOR2F, "uv_end"), "SetUvEnd",
               "GetUvEnd");

  ComponentDB::BindMethod("SetPivot", &SpriteRendererComponent::SetPivot);
  ComponentDB::BindMethod("GetPivot", &SpriteRendererComponent::Pivot);
  ADD_PROPERTY(PropertyInfo(VariantType::kVECTOR2F, "pivot"), "SetPivot",
               "GetPivot");

  ADD_PROPERTY(PropertyInfo(VariantType::kVECTOR2F, "uv_start"), "SetUvStart",
               "GetUvStart");
}
}  // namespace base_engine::component

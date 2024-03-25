// @RaytraceMaterial.h
// @brief
// @author ICE
// @date 2024/03/24
//
// @details

#pragma once
#include <variant>

#include "Material.h"
#include "Variant.h"

namespace base_engine {

struct MaterialProperty {
  std::string_view name;
  VariantType type;

  constexpr MaterialProperty(const std::string_view name,
                             const VariantType type)
      : name(name), type(type) {}
};

namespace RaytraceMaterialParameters {
constexpr MaterialProperty Diffuse = {"diffuse", VariantType::kVECTOR3F};
constexpr MaterialProperty Transparency = {"transparency", VariantType::kFloat};
constexpr MaterialProperty Specular = {"specular", VariantType::kVECTOR3F};
constexpr MaterialProperty Reflective = {"reflective", VariantType::kFloat};
constexpr MaterialProperty Roughness = {"roughness", VariantType::kFloat};
constexpr MaterialProperty Ior = {"ior", VariantType::kFloat};
constexpr MaterialProperty RayConsumption = {"rayConsumption",
                                             VariantType::kFloat};
constexpr MaterialProperty Emission = {"emission", VariantType::kVECTOR3F};

}  // namespace RaytraceMaterialParameters

class RaytraceMaterial : public RefCounted {
 public:
  void Set(const std::string& name, const Variant& value);

  bool TryGet(const std::string& name, Variant* out_result);

  struct RaytraceMaterialData {
    Vector3 diffuse;
    float transparency;
    Vector3 specular;
    float reflective;
    float roughness;
    float ior;
    float rayConsumption;
    Vector3 emission;
  } data_;

 private:
  std::vector<MaterialProperty> material_properties_ = {
      RaytraceMaterialParameters::Diffuse,
      RaytraceMaterialParameters::Transparency,
      RaytraceMaterialParameters::Specular,
      RaytraceMaterialParameters::Reflective,
      RaytraceMaterialParameters::Roughness,
      RaytraceMaterialParameters::Ior,
      RaytraceMaterialParameters::RayConsumption,
      RaytraceMaterialParameters::Emission,
  };
};
}  // namespace base_engine

#include "RaytraceMaterial.h"

namespace base_engine {
void RaytraceMaterial::Set(const std::string& name, const Variant& value) {
  size_t offset = 0;
  for (auto& [key, type] : material_properties_) {
    size_t size = 0;
    if (type == VariantType::kFloat) {
      size = sizeof(float);
    } else if (type == VariantType::kVECTOR3F) {
      size = sizeof(glm::vec3);
    } else {
      BE_ASSERT(false, "{0} Not Support Type.", name);
    }

    if (static_cast<std::string>(key.data()) != name) {
      offset += size;
    } else {
      value.Visit([&, offset, size](auto&& variant) {
        memmove_s((&data_ + offset), size, &variant, size);
      });
      break;
    }
  }
}

bool RaytraceMaterial::TryGet(const std::string& name, Variant* out_result) {
  size_t offset = 0;
  for (auto& [key, type] : material_properties_) {
    size_t size = 0;
    if (type == VariantType::kFloat) {
      size = sizeof(float);
    } else if (type == VariantType::kVECTOR3F) {
      size = sizeof(glm::vec3);
    } else {
      BE_ASSERT(false, "{0} Not Support Type.", name);
    }

    if (static_cast<std::string>(key.data()) != name) {
      offset += size;
    } else {
      *out_result = Variant{type};
      out_result->Visit([size, this, offset](auto&& v) {
        memmove_s(&v, size, (&data_ + offset), size);
      });
      return true;
    }
  }
  return false;
}
}  // namespace base_engine

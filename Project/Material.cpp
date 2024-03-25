#include "Material.h"

#include "RendererApi.h"
#include "VulkanMaterial.h"

namespace base_engine {
Ref<Material> Material::Create(const Ref<Shader>& shader,
                               const std::string& name) {
  switch (RendererApi::Current()) {
    case RendererApiType::kVulkan:
      return Ref<VulkanMaterial>::Create(shader, name);
    default:
      return nullptr;
  }
}

Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
{
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanMaterial>::Create(other, name);
  }
  return nullptr;
}
}  // namespace base_engine
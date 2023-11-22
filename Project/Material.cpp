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
}  // namespace base_engine
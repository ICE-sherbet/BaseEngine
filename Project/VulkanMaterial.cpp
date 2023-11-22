#include "VulkanMaterial.h"

#include <ranges>

namespace base_engine {
VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader,
                               const std::string& name)
    : m_Shader(shader), m_Name(name) {
  Init();
}

void VulkanMaterial::Init() { AllocateStorage(); }

void VulkanMaterial::AllocateStorage() {
  if (const auto& shader_buffers = m_Shader->GetShaderBuffers();
      !shader_buffers.empty()) {
    uint32_t size = 0;
    for (const auto& [Name, Size, Uniforms] :
         shader_buffers | std::views::values)
      size += Size;

    m_UniformStorageBuffer.Allocate(size);
    m_UniformStorageBuffer.ZeroInitialize();
  }
}
}  // namespace base_engine

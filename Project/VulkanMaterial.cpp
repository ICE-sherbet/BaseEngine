#include "VulkanMaterial.h"

#include <ranges>

#include "RendererApi.h"

namespace base_engine {
VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader,
                               const std::string& name)
    : shader_(shader), name_(name) {
  Init();
}

void VulkanMaterial::Set(const std::string& name,
                         const Ref<RendererTexture2D>& texture,
                         uint32_t arrayIndex) {
  descriptor_set_manager_.SetInput(name, texture, arrayIndex);
}

VkDescriptorSet VulkanMaterial::GetDescriptorSet(uint32_t index) {
  if (descriptor_set_manager_.GetFirstSetIndex() ==
      (std::numeric_limits<uint32_t>::max)())
    return VK_NULL_HANDLE;

  descriptor_set_manager_.InvalidateAndUpdate();

  return descriptor_set_manager_.GetDescriptorSets(index)[0];
}

Buffer VulkanMaterial::GetUniformStorageBuffer() const {
  return uniform_storage_buffer_;
}

void VulkanMaterial::Init() {
  AllocateStorage();

  DescriptorSetManagerSpecification dmSpec;
  dmSpec.DebugName = name_.empty()
                         ? fmt::format("{} (Material)", shader_->GetName())
                         : name_;
  dmSpec.Shader = shader_.As<VulkanShader>();
  dmSpec.StartSet = 0;
  dmSpec.EndSet = 0;
  dmSpec.DefaultResources = true;
  descriptor_set_manager_ = DescriptorSetManager(dmSpec);

  for (const auto& [name, decl] : descriptor_set_manager_.InputDeclarations) {
    switch (decl.Type) {
      case RenderPassInputType::ImageSampler1D:
      case RenderPassInputType::ImageSampler2D: {
        for (uint32_t i = 0; i < decl.Count; i++)
          descriptor_set_manager_.SetInput(name, Renderer::GetWhiteTexture(), i);

        break;
      }
    }
  }

  BE_CORE_VERIFY(descriptor_set_manager_.Validate());
  descriptor_set_manager_.Bake();
}

void VulkanMaterial::AllocateStorage() {
  if (const auto& shader_buffers = shader_->GetShaderBuffers();
      !shader_buffers.empty()) {
    uint32_t size = 0;
    for (const auto& [Name, Size, Uniforms] :
         shader_buffers | std::views::values)
      size += Size;

    uniform_storage_buffer_.Allocate(size);
    uniform_storage_buffer_.ZeroInitialize();
  }
}
}  // namespace base_engine

#include "DescriptorSetManager.h"

#include "RendererApi.h"

namespace base_engine {
namespace 
{
RenderPassInputType RenderPassInputTypeFromVulkanDescriptorType(
    VkDescriptorType descriptorType) {
  switch (descriptorType) {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      return RenderPassInputType::ImageSampler2D;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return RenderPassInputType::StorageImage2D;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      return RenderPassInputType::UniformBuffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return RenderPassInputType::StorageBuffer;
  }

  return RenderPassInputType::None;
}
}

DescriptorSetManager::DescriptorSetManager(
    DescriptorSetManagerSpecification specification)
    : m_Specification(std::move(specification)) {
  Init();
}

void DescriptorSetManager::Init() {
  const auto& shaderDescriptorSets =
      m_Specification.Shader->GetShaderDescriptorSets();
  uint32_t framesInFlight = 3;
  WriteDescriptorMap.resize(framesInFlight);

  for (uint32_t set = m_Specification.StartSet; set <= m_Specification.EndSet;
       set++) {
    if (set >= shaderDescriptorSets.size()) break;

    const auto& shaderDescriptor = shaderDescriptorSets[set];
    for (auto&& [bname, wd] : shaderDescriptor.WriteDescriptorSets) {

      const char* broken = strrchr(bname.c_str(), '.');
      std::string name = broken ? broken + 1 : bname;

      uint32_t binding = wd.dstBinding;
      RenderPassInputDeclaration& inputDecl = InputDeclarations[name];
      inputDecl.Type =
          RenderPassInputTypeFromVulkanDescriptorType(wd.descriptorType);
      inputDecl.Set = set;
      inputDecl.Binding = binding;
      inputDecl.Name = name;
      inputDecl.Count = wd.descriptorCount;

      if (m_Specification.DefaultResources || true) {
        RenderPassInput& input = InputResources[set][binding];
        input.Input.resize(wd.descriptorCount);
        input.Type = Utils::GetDefaultResourceType(wd.descriptorType);

        // Set default textures
        if (inputDecl.Type == RenderPassInputType::ImageSampler2D) {
          for (size_t i = 0; i < input.Input.size(); i++) {
            input.Input[i] = Renderer::GetWhiteTexture();
          }
        }
      }

      for (uint32_t frameIndex = 0; frameIndex < framesInFlight; frameIndex++)
        WriteDescriptorMap[frameIndex][set][binding] = {
            wd, std::vector<void*>(wd.descriptorCount)};
    }
  }
}
}  // namespace base_engine
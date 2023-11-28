#include "VulkanShader.h"

#include "Assert.h"
#include "Log.h"
#include "RendererApi.h"
#include "VulkanContext.h"
#include "VulkanShaderCompiler.h"

namespace base_engine {
VulkanShader::VulkanShader(const std::string& path, bool forceCompile,
                           bool disableOptimization) {
  size_t found = path.find_last_of("/\\");
  name_ = found != std::string::npos ? path.substr(found + 1) : path;
  found = name_.find_last_of('.');
  name_ = found != std::string::npos ? name_.substr(0, found) : name_;

  Reload(forceCompile);
}

void VulkanShader::Reload(bool forceCompile) {
  Renderer::Submit([instance = Ref(this), forceCompile]() mutable {
    instance->RT_Reload(forceCompile);
  });
}

void VulkanShader::RT_Reload(bool forceCompile) {
  if (!VulkanShaderCompiler::TryRecompile(this)) {
    BE_CORE_FATAL("Failed to recompile shader!");
  }
}

void VulkanShader::Release() {}

void VulkanShader::LoadAndCreateShaders(
    const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData) {
  shader_data_ = shaderData;

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  pipeline_shader_stage_create_infos_.clear();
  std::string moduleName;
  for (auto [stage, data] : shaderData) {
    VkShaderModuleCreateInfo moduleCreateInfo{};

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = data.data();

    VkShaderModule shaderModule;

    vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = shaderModule;
    shaderStage.pName = "main";
    pipeline_shader_stage_create_infos_.emplace_back(shaderStage);
  }
}

void VulkanShader::CreateDescriptors() {
  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  type_counts_.clear();
  for (uint32_t set = 0; set < reflection_data_.ShaderDescriptorSets.size();
       set++) {
    auto& shaderDescriptorSet = reflection_data_.ShaderDescriptorSets[set];

    if (!shaderDescriptorSet.UniformBuffers.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.UniformBuffers.size());
    }
    if (!shaderDescriptorSet.StorageBuffers.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.StorageBuffers.size());
    }
    if (!shaderDescriptorSet.ImageSamplers.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.ImageSamplers.size());
    }
    if (!shaderDescriptorSet.SeparateTextures.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.SeparateTextures.size());
    }
    if (!shaderDescriptorSet.SeparateSamplers.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_SAMPLER;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.SeparateSamplers.size());
    }
    if (!shaderDescriptorSet.StorageImages.empty()) {
      VkDescriptorPoolSize& typeCount = type_counts_[set].emplace_back();
      typeCount.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      typeCount.descriptorCount =
          (uint32_t)(shaderDescriptorSet.StorageImages.size());
    }

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (auto& [binding, uniformBuffer] : shaderDescriptorSet.UniformBuffers) {
      VkDescriptorSetLayoutBinding& layoutBinding =
          layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      layoutBinding.descriptorCount = 1;
      layoutBinding.stageFlags = uniformBuffer.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.binding = binding;

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[uniformBuffer.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = 1;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    for (auto& [binding, storageBuffer] : shaderDescriptorSet.StorageBuffers) {
      VkDescriptorSetLayoutBinding& layoutBinding =
          layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      layoutBinding.descriptorCount = 1;
      layoutBinding.stageFlags = storageBuffer.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.binding = binding;
      BE_CORE_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) ==
                         shaderDescriptorSet.UniformBuffers.end(),
                     "Binding is already present!");

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[storageBuffer.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = 1;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    for (auto& [binding, imageSampler] : shaderDescriptorSet.ImageSamplers) {
      auto& layoutBinding = layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      layoutBinding.descriptorCount = imageSampler.ArraySize;
      layoutBinding.stageFlags = imageSampler.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.binding = binding;

      BE_CORE_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) ==
                         shaderDescriptorSet.UniformBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) ==
                         shaderDescriptorSet.StorageBuffers.end(),
                     "Binding is already present!");

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = layoutBinding.descriptorCount;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    for (auto& [binding, imageSampler] : shaderDescriptorSet.SeparateTextures) {
      auto& layoutBinding = layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      layoutBinding.descriptorCount = imageSampler.ArraySize;
      layoutBinding.stageFlags = imageSampler.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.binding = binding;

      BE_CORE_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) ==
                         shaderDescriptorSet.UniformBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.ImageSamplers.find(binding) ==
                         shaderDescriptorSet.ImageSamplers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) ==
                         shaderDescriptorSet.StorageBuffers.end(),
                     "Binding is already present!");

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = imageSampler.ArraySize;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    for (auto& [binding, imageSampler] : shaderDescriptorSet.SeparateSamplers) {
      auto& layoutBinding = layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      layoutBinding.descriptorCount = imageSampler.ArraySize;
      layoutBinding.stageFlags = imageSampler.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.binding = binding;

      BE_CORE_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) ==
                         shaderDescriptorSet.UniformBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.ImageSamplers.find(binding) ==
                         shaderDescriptorSet.ImageSamplers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) ==
                         shaderDescriptorSet.StorageBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.SeparateTextures.find(binding) ==
                         shaderDescriptorSet.SeparateTextures.end(),
                     "Binding is already present!");

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = imageSampler.ArraySize;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    for (auto& [bindingAndSet, imageSampler] :
         shaderDescriptorSet.StorageImages) {
      auto& layoutBinding = layoutBindings.emplace_back();
      layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      layoutBinding.descriptorCount = imageSampler.ArraySize;
      layoutBinding.stageFlags = imageSampler.ShaderStage;
      layoutBinding.pImmutableSamplers = nullptr;

      uint32_t binding = bindingAndSet & 0xffffffff;
      // uint32_t descriptorSet = (bindingAndSet >> 32);
      layoutBinding.binding = binding;

      BE_CORE_ASSERT(shaderDescriptorSet.UniformBuffers.find(binding) ==
                         shaderDescriptorSet.UniformBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.StorageBuffers.find(binding) ==
                         shaderDescriptorSet.StorageBuffers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.ImageSamplers.find(binding) ==
                         shaderDescriptorSet.ImageSamplers.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.SeparateTextures.find(binding) ==
                         shaderDescriptorSet.SeparateTextures.end(),
                     "Binding is already present!");
      BE_CORE_ASSERT(shaderDescriptorSet.SeparateSamplers.find(binding) ==
                         shaderDescriptorSet.SeparateSamplers.end(),
                     "Binding is already present!");

      VkWriteDescriptorSet& writeDescriptorSet =
          shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name];
      writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
      writeDescriptorSet.descriptorCount = layoutBinding.descriptorCount;
      writeDescriptorSet.dstBinding = layoutBinding.binding;
    }

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = (uint32_t)(layoutBindings.size());
    descriptorLayout.pBindings = layoutBindings.data();

    BE_CORE_INFO_TAG(
        "Renderer",
        "Creating descriptor set {0} with {1} ubo's, {2} ssbo's, {3} samplers, "
        "{4} separate textures, {5} separate samplers and {6} storage images",
        set, shaderDescriptorSet.UniformBuffers.size(),
        shaderDescriptorSet.StorageBuffers.size(),
        shaderDescriptorSet.ImageSamplers.size(),
        shaderDescriptorSet.SeparateTextures.size(),
        shaderDescriptorSet.SeparateSamplers.size(),
        shaderDescriptorSet.StorageImages.size());
    if (set >= descriptor_set_layouts_.size())
      descriptor_set_layouts_.resize((size_t)(set + 1));
    vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr,
                                &descriptor_set_layouts_[set]);
  }
}

std::vector<VkDescriptorSetLayout> VulkanShader::GetAllDescriptorSetLayouts()
{
	std::vector<VkDescriptorSetLayout> result;
	result.reserve(descriptor_set_layouts_.size());
	for (auto& layout : descriptor_set_layouts_) result.emplace_back(layout);

	return result;
}

void VulkanShader::SetReflectionData(const ReflectionData& reflection_data) {
  reflection_data_ = reflection_data;
}
}  // namespace base_engine

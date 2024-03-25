// @VulkanShaderResource.h
// @brief
// @author ICE
// @date 2023/11/20
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include <string>
#include <unordered_map>

#include "VulkanAllocator.h"

namespace base_engine {
namespace shader {
struct UniformBuffer {
  VkDescriptorBufferInfo Descriptor;
  uint32_t Size = 0;
  uint32_t BindingPoint = 0;
  std::string Name;
  VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct StorageBuffer {
  VmaAllocation MemoryAlloc = nullptr;
  VkDescriptorBufferInfo Descriptor;
  uint32_t Size = 0;
  uint32_t BindingPoint = 0;
  std::string Name;
  VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct ImageSampler {
  uint32_t BindingPoint = 0;
  uint32_t DescriptorSet = 0;
  uint32_t Dimension = 0;
  uint32_t ArraySize = 0;
  std::string Name;
  VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct PushConstantRange {
  VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  uint32_t Offset = 0;
  uint32_t Size = 0;
};

struct AccelerationStructureBuffer
{
	uint32_t BindingPoint = 0;
  uint32_t ArraySize = 0;
  std::string Name;
	VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

struct ShaderDescriptorSet {
  std::unordered_map<uint32_t, UniformBuffer> UniformBuffers;
  std::unordered_map<uint32_t, StorageBuffer> StorageBuffers;
  std::unordered_map<uint32_t, ImageSampler> ImageSamplers;
  std::unordered_map<uint32_t, ImageSampler> StorageImages;
  std::unordered_map<uint32_t, ImageSampler> SeparateTextures;
  std::unordered_map<uint32_t, ImageSampler> SeparateSamplers;
  
  std::unordered_map<uint32_t, AccelerationStructureBuffer>
      AccelerationStructures;

  std::unordered_map<std::string, VkWriteDescriptorSet> WriteDescriptorSets;

  explicit operator bool() const {
    return !(StorageBuffers.empty() && UniformBuffers.empty() &&
             ImageSamplers.empty() && StorageImages.empty() &&
             SeparateTextures.empty() && SeparateSamplers.empty()
      && AccelerationStructures.empty());
  }
};
}  // namespace shader

}  // namespace base_engine

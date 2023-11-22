// @VulkanAPI.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include "RendererStats.h"
#include "VulkanContext.h"
#include "VulkanShaderCompiler.h"

namespace base_engine::vulkan {
static VkDescriptorSetAllocateInfo DescriptorSetAllocInfo(
    const VkDescriptorSetLayout* layouts, const uint32_t count,
    const VkDescriptorPool pool) {
  VkDescriptorSetAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.pSetLayouts = layouts;
  info.descriptorSetCount = count;
  info.descriptorPool = pool;
  return info;
}

static VkSampler CreateSampler(const VkSamplerCreateInfo& samplerCreateInfo) {
  auto device = VulkanContext::GetCurrentDevice();
  const VkDevice vulkan_device = device->GetVulkanDevice();

  VkSampler sampler;

  vkCreateSampler(vulkan_device, &samplerCreateInfo, nullptr, &sampler);

  renderer_utils::GetResourceAllocationCounts().samplers++;

  return sampler;
}

static void DestroySampler(const VkSampler sampler) {
  auto device = VulkanContext::GetCurrentDevice();
  const VkDevice vulkan_device = device->GetVulkanDevice();
  vkDestroySampler(vulkan_device, sampler, nullptr);

  renderer_utils::GetResourceAllocationCounts().samplers--;
}
}  // namespace base_engine::vulkan

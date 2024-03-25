// @VulkanRaytraceRenderPipeline.h
// @brief
// @author ICE
// @date 2024/03/14
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include "Pipeline.h"
#include "VulkanBuffer.h"

namespace base_engine {
class VulkanRaytracePipeline : public RefCounted {
 public:
  explicit VulkanRaytracePipeline(const PipelineSpecification& spec);

  PipelineSpecification& GetSpecification();
  void Invalidate();
  Ref<Shader> GetShader() const;

  void CreatePipeline(VkDescriptorSetLayout layout);

private:
  VkPhysicalDeviceRayTracingPipelinePropertiesKHR pipeline_properties_;

  VkPipelineLayout m_PipelineLayout = nullptr;
  VkPipeline m_VulkanPipeline = nullptr;
  VkPipelineCache m_PipelineCache = nullptr;

  VkStridedDeviceAddressRegionKHR m_rgenRegion{};
  VkStridedDeviceAddressRegionKHR m_missRegion{};
  VkStridedDeviceAddressRegionKHR m_hitRegion{};
  VkStridedDeviceAddressRegionKHR m_callRegion{};

  PipelineSpecification spec_;

  Ref<VulkanBuffer> m_sbtBuffer;
};
}  // namespace base_engine

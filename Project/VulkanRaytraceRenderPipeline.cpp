#include "VulkanRaytraceRenderPipeline.h"

#include <vulkan/vulkan.h>

#include "RendererApi.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanShaderCompiler.h"

namespace base_engine {

template <typename T>
constexpr T AlignUp(T x, size_t a) noexcept {
  return T((x + (T(a) - 1)) & ~T(a - 1));
}

inline VkRayTracingShaderGroupCreateInfoKHR generalShaderGroupInfo(
    uint32_t index) {
  VkRayTracingShaderGroupCreateInfoKHR info = {
      VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
  info.generalShader = index;
  info.closestHitShader = VK_SHADER_UNUSED_KHR;
  info.anyHitShader = VK_SHADER_UNUSED_KHR;
  info.intersectionShader = VK_SHADER_UNUSED_KHR;

  return info;
}

inline VkRayTracingShaderGroupCreateInfoKHR closestHitShaderGroupInfo(
    uint32_t index) {
  VkRayTracingShaderGroupCreateInfoKHR info = {
      VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
  info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
  info.generalShader = VK_SHADER_UNUSED_KHR;
  info.closestHitShader = index;
  info.anyHitShader = VK_SHADER_UNUSED_KHR;
  info.intersectionShader = VK_SHADER_UNUSED_KHR;

  return info;
}

VulkanRaytracePipeline::VulkanRaytracePipeline(
    const PipelineSpecification& spec) {
  spec_ = spec;
  pipeline_properties_ = {
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
  auto device = VulkanContext::Get()->GetDevice();
  auto physical_device_ = device->GetVulkanPhysicalDevice();
  VkPhysicalDeviceProperties2 prop2{
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
  prop2.pNext = &pipeline_properties_;
  vkGetPhysicalDeviceProperties2(physical_device_, &prop2);
}

PipelineSpecification& VulkanRaytracePipeline::GetSpecification() {
  return spec_;
}

Ref<Shader> VulkanRaytracePipeline::GetShader() const { return spec_.Shader; }

void VulkanRaytracePipeline::CreatePipeline(
    const VkDescriptorSetLayout layout) {
  auto vkDevice = VulkanContext::Get()->GetDevice()->GetVulkanDevice();

  auto raygenShader =
      VulkanShaderCompiler::Compile("raygen.rgen", false, false);
  auto missShaders1 = VulkanShaderCompiler::Compile("miss.rmiss", false, false);
  auto missShaders2 =
      VulkanShaderCompiler::Compile("shadowMiss.rmiss", false, false);
  auto closestHitShaders =
      VulkanShaderCompiler::Compile("closesthit.rchit", false, false);

  const auto groupSize = AlignUp(pipeline_properties_.shaderGroupHandleSize,
                                 pipeline_properties_.shaderGroupBaseAlignment);
  const auto groupStride = groupSize;

  std::vector<VkPipelineShaderStageCreateInfo> stages;
  std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

  {
    m_rgenRegion.deviceAddress = (groups.size() * groupSize);
    const auto stage = raygenShader->GetPipelineShaderStageCreateInfos()[0];
    stages.push_back(stage);
    groups.push_back(generalShaderGroupInfo((uint32_t)groups.size()));

    m_rgenRegion.stride = groupStride;
    m_rgenRegion.size = groupSize;
  }

  {
    m_missRegion.deviceAddress = (groups.size() * groupSize);

    auto stage = missShaders1->GetPipelineShaderStageCreateInfos()[0];
    stages.push_back(stage);
    groups.push_back(generalShaderGroupInfo((uint32_t)groups.size()));

    stage = missShaders2->GetPipelineShaderStageCreateInfos()[0];
    stages.push_back(stage);
    groups.push_back(generalShaderGroupInfo((uint32_t)groups.size()));

    m_missRegion.stride = groupStride;
    m_missRegion.size = groupSize;
  }

  {
    m_hitRegion.deviceAddress = (groups.size() * groupSize);

    const auto stage =
        closestHitShaders->GetPipelineShaderStageCreateInfos()[0];
    stages.push_back(stage);
    groups.push_back(closestHitShaderGroupInfo((uint32_t)groups.size()));

    m_hitRegion.stride = groupStride;
    m_hitRegion.size = groupSize;
  }

  {
    VkPipelineLayoutCreateInfo info = {};
    info.setLayoutCount = 1;
    info.pSetLayouts = &layout;

    vkCreatePipelineLayout(vkDevice, &info, nullptr, &m_PipelineLayout);
  }

  {
    VkRayTracingPipelineCreateInfoKHR info = {};
    info.stageCount = static_cast<uint32_t>(stages.size());
    info.pStages = stages.data();
    info.groupCount = static_cast<uint32_t>(groups.size());
    info.pGroups = groups.data();
    info.maxPipelineRayRecursionDepth = 7;
    info.layout = m_PipelineLayout;

    vkCreateRayTracingPipelinesKHR(vkDevice, nullptr, {}, 1, &info, nullptr,
                                   &m_VulkanPipeline);
  }

  // shader binding table
  {
    const auto groupCount = groups.size();
    const auto sbtSize = groupCount * groupSize;

    std::vector<uint8_t> groupHandles(
        groupCount * pipeline_properties_.shaderGroupHandleSize);

    {
      const auto result = vkGetRayTracingShaderGroupHandlesKHR(
          vkDevice, m_VulkanPipeline, 0, (uint32_t)groupCount,
          groupHandles.size(), groupHandles.data());
      if (result != VK_SUCCESS) {
        BE_FATAL("Unable to get ray tracing shader group handles");
      }
    }

    m_sbtBuffer = Ref<VulkanBuffer>::Create(
        "Shader Binding Table", sbtSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, false);

    // Shader group handles should be aligned according to
    // shaderGroupBaseAlignment. The handles we get from
    // getRayTracingShaderGroupHandlesKHR are consecutive in memory
    // (shaderGroupHandleSize), hence we need to copy them over adjusting the
    // alignment.
    {
      const auto groupSizeAligned =
          AlignUp(pipeline_properties_.shaderGroupHandleSize,
                  pipeline_properties_.shaderGroupBaseAlignment);

      auto p = reinterpret_cast<uint8_t*>(m_sbtBuffer->Map());
      for (auto i = 0u; i < groupCount; i++) {
        memcpy(p,
               groupHandles.data() +
                   i * pipeline_properties_.shaderGroupHandleSize,
               pipeline_properties_.shaderGroupHandleSize);
        p += groupSizeAligned;
      }

      m_sbtBuffer->Unmap();
    }

    m_rgenRegion.deviceAddress =
        m_sbtBuffer->Address() + m_rgenRegion.deviceAddress;
    m_missRegion.deviceAddress =
        m_sbtBuffer->Address() + m_missRegion.deviceAddress;
    m_hitRegion.deviceAddress =
        m_sbtBuffer->Address() + m_hitRegion.deviceAddress;
  }
}

void VulkanRaytracePipeline::Invalidate() {
#if 0
  // pipeline layout
  {
    VkPipelineLayoutCreateInfo info = {};
    info.setLayoutCount =1;
    info.pSetLayouts = &m_descriptorSet.layout();

    m_pipelineLayout = vc.device->createPipelineLayoutUnique(info);
    vc.setObjectName(*m_pipelineLayout, "Ray Tracer");
  }

  // pipeline
  {
    vk::RayTracingPipelineCreateInfoKHR info = {};
    info.setStageCount((uint32_t)stages.size());
    info.setPStages(stages.data());
    info.setGroupCount((uint32_t)groups.size());
    info.setPGroups(groups.data());
    info.setMaxPipelineRayRecursionDepth(7);
    info.setLayout(*m_pipelineLayout);

    m_pipeline =
        vc.device->createRayTracingPipelineKHRUnique(nullptr, nullptr, info)
            .value;
    vc.setObjectName(*m_pipeline, "Ray Tracer");
  }
#endif
}
}  // namespace base_engine

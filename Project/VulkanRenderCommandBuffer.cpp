#include "VulkanRenderCommandBuffer.h"

#include "Application.h"
#include "Platform/Windows/Window.h"
#include "RendererApi.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanUtilities.h"

namespace base_engine {
VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(uint32_t count,
                                                     std::string debugName)
    : debug_name_(std::move(debugName)) {
  auto device = VulkanContext::GetCurrentDevice();
  uint32_t framesInFlight = 3;

  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex =
      device->GetPhysicalDevice()->GetIndices().graphics;
  cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCreateCommandPool(device->GetVulkanDevice(), &cmdPoolInfo, nullptr,
                      &command_pool_);
  vulkan::SetDebugUtilsObjectName(device->GetVulkanDevice(),
                                  VK_OBJECT_TYPE_COMMAND_POOL, debug_name_,
                                  command_pool_);

  VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
  commandBufferAllocateInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  commandBufferAllocateInfo.commandPool = command_pool_;
  commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  if (count == 0) count = framesInFlight;
  commandBufferAllocateInfo.commandBufferCount = count;
  command_buffers_.resize(count);
  vkAllocateCommandBuffers(device->GetVulkanDevice(),
                           &commandBufferAllocateInfo, command_buffers_.data());

  for (uint32_t i = 0; i < count; ++i)
    vulkan::SetDebugUtilsObjectName(
        device->GetVulkanDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER,
        fmt::format("{} (frame in flight: {})", debug_name_, i),
        command_buffers_[i]);

  VkFenceCreateInfo fenceCreateInfo{};
  fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  wait_fences_.resize(framesInFlight);
  for (size_t i = 0; i < wait_fences_.size(); ++i) {
    vkCreateFence(device->GetVulkanDevice(), &fenceCreateInfo, nullptr,
                  &wait_fences_[i]);
    vulkan::SetDebugUtilsObjectName(
        device->GetVulkanDevice(), VK_OBJECT_TYPE_FENCE,
        fmt::format("{} (frame in flight: {}) fence", debug_name_, i),
        wait_fences_[i]);
  }

  VkQueryPoolCreateInfo queryPoolCreateInfo = {};
  queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
  queryPoolCreateInfo.pNext = nullptr;

  // Timestamp queries
  const uint32_t maxUserQueries = 16;
  timestamp_query_count_ = 2 + 2 * maxUserQueries;

  queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
  queryPoolCreateInfo.queryCount = timestamp_query_count_;
  timestamp_query_pools_.resize(framesInFlight);
  for (auto& timestampQueryPool : timestamp_query_pools_)
    vkCreateQueryPool(device->GetVulkanDevice(), &queryPoolCreateInfo, nullptr,
                      &timestampQueryPool);

  timestamp_query_results_.resize(framesInFlight);
  for (auto& timestampQueryResults : timestamp_query_results_)
    timestampQueryResults.resize(timestamp_query_count_);

  execution_gpu_times_.resize(framesInFlight);
  for (auto& executionGPUTimes : execution_gpu_times_)
    executionGPUTimes.resize(timestamp_query_count_ / 2);

  // Pipeline statistics queries
  pipeline_query_count_ = 7;
  queryPoolCreateInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
  queryPoolCreateInfo.queryCount = pipeline_query_count_;
  queryPoolCreateInfo.pipelineStatistics =
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

  pipeline_statistics_query_pools_.resize(framesInFlight);
  for (auto& pipelineStatisticsQueryPools : pipeline_statistics_query_pools_)
    vkCreateQueryPool(device->GetVulkanDevice(), &queryPoolCreateInfo, nullptr,
                      &pipelineStatisticsQueryPools);
}

VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer() {
  if (owned_by_swap_chain_) return;

  VkCommandPool commandPool = command_pool_;
  Renderer::SubmitResourceFree([commandPool]() {
    auto device = VulkanContext::GetCurrentDevice();
    vkDestroyCommandPool(device->GetVulkanDevice(), commandPool, nullptr);
  });
}

void VulkanRenderCommandBuffer::Begin() {
  timestamp_next_available_query_ = 2;

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    const uint32_t frame_index = Renderer::RT_GetCurrentFrameIndex();

    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufInfo.pNext = nullptr;

    VkCommandBuffer command_buffer;
    if (instance->owned_by_swap_chain_) {
      VulkanSwapChain& swap_chain =
          static_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
      command_buffer = swap_chain.GetDrawCommandBuffer(frame_index);
    } else {
      command_buffer = instance->command_buffers_[frame_index];
    }
    instance->active_command_buffer_ = command_buffer;
    auto r = vkBeginCommandBuffer(command_buffer, &cmdBufInfo);
  });
}

void VulkanRenderCommandBuffer::End() {
  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    uint32_t frameIndex = Renderer::RT_GetCurrentFrameIndex();
    VkCommandBuffer commandBuffer = instance->active_command_buffer_;

    auto r = vkEndCommandBuffer(commandBuffer);

    instance->active_command_buffer_ = nullptr;
  });
}

void VulkanRenderCommandBuffer::Submit() {
  if (owned_by_swap_chain_) return;

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();

    const uint32_t frame_index = Renderer::RT_GetCurrentFrameIndex();

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    const VkCommandBuffer command_buffer =
        instance->command_buffers_[frame_index];
    submit_info.pCommandBuffers = &command_buffer;

    auto r = vkWaitForFences(device->GetVulkanDevice(), 1,
                    &instance->wait_fences_[frame_index], VK_TRUE, UINT64_MAX);
    r = vkResetFences(device->GetVulkanDevice(), 1,
                  &instance->wait_fences_[frame_index]);
    auto graphicsQueue = device->GetGraphicsQueue();

    r = vkQueueSubmit(graphicsQueue, 1, &submit_info,
                  instance->wait_fences_[frame_index]);
  });
}
}  // namespace base_engine

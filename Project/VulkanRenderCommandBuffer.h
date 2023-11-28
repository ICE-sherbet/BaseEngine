// @VulkanRenderCommandBuffer.h
// @brief
// @author ICE
// @date 2023/11/23
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include <vector>

#include "RenderCommandBuffer.h"

namespace base_engine {

struct PipelineStatistics {
  uint64_t InputAssemblyVertices = 0;
  uint64_t InputAssemblyPrimitives = 0;
  uint64_t VertexShaderInvocations = 0;
  uint64_t ClippingInvocations = 0;
  uint64_t ClippingPrimitives = 0;
  uint64_t FragmentShaderInvocations = 0;
  uint64_t ComputeShaderInvocations = 0;
};
class VulkanRenderCommandBuffer : public RenderCommandBuffer {
 public:
  explicit VulkanRenderCommandBuffer(uint32_t count = 0,
                                     std::string debugName = "");
  ~VulkanRenderCommandBuffer() override;

  void Begin() override;
  void End() override;
  void Submit() override;

  VkCommandBuffer GetActiveCommandBuffer() const {
    return active_command_buffer_;
  }

 private:
  std::string debug_name_;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers_;
  VkCommandBuffer active_command_buffer_ = nullptr;
  std::vector<VkFence> wait_fences_;

  bool owned_by_swap_chain_ = false;

  uint32_t timestamp_query_count_ = 0;
  uint32_t timestamp_next_available_query_ = 2;
  std::vector<VkQueryPool> timestamp_query_pools_;
  std::vector<VkQueryPool> pipeline_statistics_query_pools_;
  std::vector<std::vector<uint64_t>> timestamp_query_results_;
  std::vector<std::vector<float>> execution_gpu_times_;

  uint32_t pipeline_query_count_ = 0;
};
}  // namespace base_engine

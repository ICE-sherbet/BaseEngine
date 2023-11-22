// @VulkanUniformBuffer.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <string>
#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#include "UniformBuffer.h"

namespace base_engine {
class VulkanUniformBuffer : public UniformBuffer {
 public:
	explicit VulkanUniformBuffer(uint32_t size);
	~VulkanUniformBuffer() override;

  void SetData(const void* data, uint32_t size,
                       uint32_t offset = 0) override;
  void RT_SetData(const void* data, uint32_t size,
                          uint32_t offset = 0) override;

  const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const {
    return descriptor_buffer_info_;
  }

 private:
  void Release();
  void RT_Invalidate();

 private:
  VmaAllocation memory_alloc_ = nullptr;
  VkBuffer buffer_;
  VkDescriptorBufferInfo descriptor_buffer_info_{};
  uint32_t size_ = 0;
  std::string name_;
  VkShaderStageFlagBits shader_stage_ = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

  uint8_t* local_storage_ = nullptr;

};
}  // namespace base_engine

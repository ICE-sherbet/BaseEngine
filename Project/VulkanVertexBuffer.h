// @VulkanVertexBuffer.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "VertexBuffer.h"

namespace base_engine {
class VulkanVertexBuffer : public VertexBuffer {
 public:
  VulkanVertexBuffer(void* data, uint32_t size,
                     VertexBufferUsage usage = VertexBufferUsage::kStatic);
  VulkanVertexBuffer(uint32_t size,
                     VertexBufferUsage usage = VertexBufferUsage::kDynamic);

  ~VulkanVertexBuffer() override;

  void SetData(void* buffer, uint64_t size, uint64_t offset = 0) override;
  void RT_SetData(void* buffer, uint64_t size, uint64_t offset = 0) override;
  unsigned int GetSize() const override { return size_; }

  VkBuffer GetVulkanBuffer() const { return vulkan_buffer_; }

 private:
  uint32_t size_ = 0;
  Buffer local_data_;

  VkBuffer vulkan_buffer_ = VK_NULL_HANDLE;
  VmaAllocation memory_allocation_;
};
}  // namespace base_engine

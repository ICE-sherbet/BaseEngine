// @VulkanIndexBuffer.h
// @brief
// @author ICE
// @date 2023/11/27
//
// @details

#pragma once
#include "Buffer.h"
#include "IndexBuffer.h"
#include "VulkanAllocator.h"

namespace base_engine {
class VulkanIndexBuffer : public IndexBuffer {
 public:
  VulkanIndexBuffer(uint64_t size);
  VulkanIndexBuffer(void* data, uint64_t size = 0);
  virtual ~VulkanIndexBuffer();

  uint32_t GetCount() const override { return size_ / sizeof(uint32_t); }

  uint64_t GetSize() const override { return size_; }

  VkBuffer GetVulkanBuffer() { return vulkan_buffer_; }

 private:
  uint64_t size_ = 0;
  Buffer local_data_;

  VkBuffer vulkan_buffer_ = VK_NULL_HANDLE;
  VmaAllocation memory_allocation_;
};
}  // namespace base_engine

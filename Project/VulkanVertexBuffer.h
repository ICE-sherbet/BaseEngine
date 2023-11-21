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

  void SetData(void* buffer, uint64_t size,
               uint64_t offset = 0) override;
  void RT_SetData(void* buffer, uint64_t size,
                  uint64_t offset = 0) override;
  unsigned int GetSize() const override { return m_Size; }

  VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }

 private:
  uint32_t m_Size = 0;
  Buffer m_LocalData;

  VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
  VmaAllocation m_MemoryAllocation;
};
}  // namespace base_engine

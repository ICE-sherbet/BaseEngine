// @VulkanVertexBuffer.h
// @brief
// @author ICE
// @date 2023/11/06
//
// @details

#pragma once
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "VertexBuffer.h"

namespace base_engine {
class VulkanVertexBuffer : VertexBuffer {
 public:
  VulkanVertexBuffer(void* data, uint32_t size,
                     VertexBufferUsage usage = VertexBufferUsage::kStatic);
  VulkanVertexBuffer(uint32_t size,
                     VertexBufferUsage usage = VertexBufferUsage::kDynamic) {}

  virtual ~VulkanVertexBuffer() override {}

  virtual void SetData(void* buffer, uint32_t size,
                       uint32_t offset = 0) override {}
  virtual void RT_SetData(void* buffer, uint32_t size,
                          uint32_t offset = 0) override {}
  virtual void Bind() const override {}

  virtual unsigned int GetSize() const override { return m_Size; }

  VkBuffer GetVulkanBuffer() const { return m_VulkanBuffer; }

 private:
  uint32_t m_Size = 0;

  VkBuffer m_VulkanBuffer = VK_NULL_HANDLE;
  VmaAllocation m_MemoryAllocation;
};
}  // namespace base_engine

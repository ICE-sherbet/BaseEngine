#include "VulkanUniformBuffer.h"

#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace base_engine {
VulkanUniformBuffer::VulkanUniformBuffer(const uint32_t size) : size_(size) {
  local_storage_ = new uint8_t[size];


  RT_Invalidate();
}

VulkanUniformBuffer::~VulkanUniformBuffer() { Release(); }

void VulkanUniformBuffer::Release() {
  if (!memory_alloc_) return;

  Renderer::SubmitResourceFree(
      [buffer = buffer_, memoryAlloc = memory_alloc_]() {
        VulkanAllocator allocator("UniformBuffer");
        allocator.DestroyBuffer(buffer, memoryAlloc);
      });

  buffer_ = VK_NULL_HANDLE;
  memory_alloc_ = nullptr;

  delete[] local_storage_;
  local_storage_ = nullptr;
}

void VulkanUniformBuffer::RT_Invalidate() {
  Release();

  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  bufferInfo.size = size_;

  VulkanAllocator allocator("UniformBuffer");
  memory_alloc_ = allocator.AllocateBuffer(
      bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, buffer_);

  descriptor_buffer_info_.buffer = buffer_;
  descriptor_buffer_info_.offset = 0;
  descriptor_buffer_info_.range = size_;
}

void VulkanUniformBuffer::SetData(const void* data, uint32_t size,
                                  uint32_t offset) {
  memcpy(local_storage_, data, size);
  Ref instance = this;
  Renderer::Submit([instance, size, offset]() mutable {
    instance->RT_SetData(instance->local_storage_, size, offset);
  });
}

void VulkanUniformBuffer::RT_SetData(const void* data, uint32_t size,
                                     uint32_t offset) {
  VulkanAllocator allocator("VulkanUniformBuffer");
  uint8_t* pData = allocator.MapMemory<uint8_t>(memory_alloc_);
  memcpy(pData, (const uint8_t*)data + offset, size);
  allocator.UnmapMemory(memory_alloc_);
}

}  // namespace base_engine

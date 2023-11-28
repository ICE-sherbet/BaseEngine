#include "VulkanVertexBuffer.h"

#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace base_engine {
VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t size,
                                       VertexBufferUsage usage)
    : size_(size) {
  local_data_ = Buffer::Copy(data, size);

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VulkanAllocator allocator("VertexBuffer");

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = instance->size_;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer staging_buffer;
    const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

    uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
    memcpy(destData, instance->local_data_.Data, instance->local_data_.Size);
    allocator.UnmapMemory(stagingBufferAllocation);

    VkBufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.size = instance->size_;
    vertexBufferCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    instance->memory_allocation_ = allocator.AllocateBuffer(
        vertexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY,
        instance->vulkan_buffer_);

    const VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

    VkBufferCopy copy_region = {};
    copy_region.size = instance->local_data_.Size;
    vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->vulkan_buffer_, 1,
                    &copy_region);

    device->FlushCommandBuffer(copy_cmd);

    allocator.DestroyBuffer(staging_buffer, stagingBufferAllocation);
  });
}
VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage)
    : size_(size) {
  local_data_.Allocate(size);

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VulkanAllocator allocator("VertexBuffer");

    VkBufferCreateInfo vertex_buffer_create_info = {};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.size = instance->size_;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    instance->memory_allocation_ = allocator.AllocateBuffer(
        vertex_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU,
        instance->vulkan_buffer_);
  });
}

VulkanVertexBuffer::~VulkanVertexBuffer() {
  VkBuffer buffer = vulkan_buffer_;
  VmaAllocation allocation = memory_allocation_;
  Renderer::SubmitResourceFree([buffer, allocation]() {
    VulkanAllocator allocator("VertexBuffer");
    allocator.DestroyBuffer(buffer, allocation);
  });

  local_data_.Release();
}

void VulkanVertexBuffer::SetData(void* buffer, uint64_t size, uint64_t offset) {
  memcpy(local_data_.Data, (uint8_t*)buffer + offset, size);
  Ref instance = this;
  Renderer::Submit([instance, size, offset]() mutable {
    instance->RT_SetData(instance->local_data_.Data, size, offset);
  });
}

void VulkanVertexBuffer::RT_SetData(void* buffer, uint64_t size,
                                    uint64_t offset) {
  VulkanAllocator allocator("VulkanVertexBuffer");
  uint8_t* pData = allocator.MapMemory<uint8_t>(memory_allocation_);
  memcpy(pData, (uint8_t*)buffer + offset, size);
  allocator.UnmapMemory(memory_allocation_);
}
}  // namespace base_engine

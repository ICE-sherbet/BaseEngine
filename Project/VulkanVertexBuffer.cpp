#include "VulkanVertexBuffer.h"

#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace base_engine {
VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t size,
                                       VertexBufferUsage usage)
    : m_Size(size) {
  m_LocalData = Buffer::Copy(data, size);

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VulkanAllocator allocator("VertexBuffer");

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = instance->m_Size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer staging_buffer;
    const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

    uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
    memcpy(destData, instance->m_LocalData.Data, instance->m_LocalData.Size);
    allocator.UnmapMemory(stagingBufferAllocation);

    VkBufferCreateInfo vertexBufferCreateInfo = {};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.size = instance->m_Size;
    vertexBufferCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    instance->m_MemoryAllocation = allocator.AllocateBuffer(
        vertexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY,
        instance->m_VulkanBuffer);

    const VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

    VkBufferCopy copy_region = {};
    copy_region.size = instance->m_LocalData.Size;
    vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->m_VulkanBuffer, 1,
                    &copy_region);

    device->FlushCommandBuffer(copy_cmd);

    allocator.DestroyBuffer(staging_buffer, stagingBufferAllocation);
  });
}
VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage)
    : m_Size(size) {
  m_LocalData.Allocate(size);

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VulkanAllocator allocator("VertexBuffer");

    VkBufferCreateInfo vertex_buffer_create_info = {};
    vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_create_info.size = instance->m_Size;
    vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    instance->m_MemoryAllocation = allocator.AllocateBuffer(
        vertex_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU,
        instance->m_VulkanBuffer);
  });
}

VulkanVertexBuffer::~VulkanVertexBuffer() {
  VkBuffer buffer = m_VulkanBuffer;
  VmaAllocation allocation = m_MemoryAllocation;
  Renderer::SubmitResourceFree([buffer, allocation]() {
    VulkanAllocator allocator("VertexBuffer");
    allocator.DestroyBuffer(buffer, allocation);
  });

  m_LocalData.Release();
}

void VulkanVertexBuffer::SetData(void* buffer, uint64_t size, uint64_t offset) {
  memcpy(m_LocalData.Data, (uint8_t*)buffer + offset, size);
  Ref instance = this;
  Renderer::Submit([instance, size, offset]() mutable {
    instance->RT_SetData(instance->m_LocalData.Data, size, offset);
  });
}

void VulkanVertexBuffer::RT_SetData(void* buffer, uint64_t size,
                                    uint64_t offset) {
  VulkanAllocator allocator("VulkanVertexBuffer");
  uint8_t* pData = allocator.MapMemory<uint8_t>(m_MemoryAllocation);
  memcpy(pData, (uint8_t*)buffer + offset, size);
  allocator.UnmapMemory(m_MemoryAllocation);
}
}  // namespace base_engine

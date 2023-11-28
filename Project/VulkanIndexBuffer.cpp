#include "VulkanIndexBuffer.h"

#include "RendererApi.h"
#include "VulkanContext.h"

namespace base_engine {

VulkanIndexBuffer::VulkanIndexBuffer(uint64_t size) : size_(size) {}

VulkanIndexBuffer::VulkanIndexBuffer(void* data, uint64_t size) : size_(size) {
  local_data_ = Buffer::Copy(data, size);

  Ref instance = this;
  Renderer::Submit([instance]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VulkanAllocator allocator("IndexBuffer");

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = instance->size_;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_allocation = allocator.AllocateBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

    // Copy data to staging buffer
    uint8_t* destData = allocator.MapMemory<uint8_t>(staging_buffer_allocation);
    memcpy(destData, instance->local_data_.Data, instance->local_data_.Size);
    allocator.UnmapMemory(staging_buffer_allocation);

    VkBufferCreateInfo indexBufferCreateInfo = {};
    indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferCreateInfo.size = instance->size_;
    indexBufferCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    instance->memory_allocation_ = allocator.AllocateBuffer(
        indexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY,
        instance->vulkan_buffer_);

    const VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

    VkBufferCopy copy_region = {};
    copy_region.size = instance->local_data_.Size;
    vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->vulkan_buffer_, 1,
                    &copy_region);

    device->FlushCommandBuffer(copy_cmd);

    allocator.DestroyBuffer(staging_buffer, staging_buffer_allocation);
  });
}

VulkanIndexBuffer::~VulkanIndexBuffer() {
  VkBuffer buffer = vulkan_buffer_;
  VmaAllocation allocation = memory_allocation_;
  Renderer::SubmitResourceFree([buffer, allocation]() {
    VulkanAllocator allocator("IndexBuffer");
    allocator.DestroyBuffer(buffer, allocation);
  });
  local_data_.Release();
}
}  // namespace base_engine

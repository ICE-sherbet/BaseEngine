#include "VulkanBuffer.h"

#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"

namespace base_engine {
VulkanBuffer::VulkanBuffer(const std::string& name, const void* data, uint32_t size,
                           VkBufferUsageFlags usage_flags, bool RT) {
  local_data_ = Buffer::Copy(data, size);
  descriptor_info_.range = size;
  name_ = name;
  if (RT) {
    Ref instance = this;
    Renderer::Submit([instance, usage_flags]() mutable {
      instance->RT_CreateBuffer(instance->local_data_.Data,
                                instance->local_data_.Size, usage_flags);
    });
  } else {
    RT_CreateBuffer(local_data_.Data, local_data_.Size, usage_flags);
  }
}

VulkanBuffer::VulkanBuffer(const std::string& name, VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags memory_type, bool RT) {
  descriptor_info_.range = size;
  local_data_.Allocate(size);
  name_ = name;
  if (RT) {
    Ref instance = this;
    Renderer::Submit([instance, usage]() mutable {
      instance->RT_CreateBuffer(instance->local_data_.Size, usage);
    });
  } else {
    RT_CreateBuffer(local_data_.Size, usage);
  }
}

VulkanBuffer::~VulkanBuffer() {
  VkBuffer buffer = buffer_;
  VmaAllocation allocation = memory_allocation_;
  std::string name = name_;
  Renderer::SubmitResourceFree([buffer, allocation, name]() {
    VulkanAllocator allocator(name);
    allocator.DestroyBuffer(buffer, allocation);
  });

  local_data_.Release();
}

VulkanBuffer::operator VkBuffer_T*() const { return buffer_; }

VkDeviceSize VulkanBuffer::size() const { return descriptor_info_.range; }
const VkDescriptorBufferInfo& VulkanBuffer::DescriptorInfo() const {
  return descriptor_info_;
}

void* VulkanBuffer::Map()
{
  VulkanAllocator allocator(name_);

  return allocator.MapMemory<uint8_t>(memory_allocation_);
}
void VulkanBuffer::Unmap() {
  VulkanAllocator allocator(name_);
  allocator.UnmapMemory(memory_allocation_);
}
void VulkanBuffer::SetData(void* buffer, uint64_t size, uint64_t offset) {
  memcpy(local_data_.Data, (uint8_t*)buffer + offset, size);
  Ref instance = this;
  Renderer::Submit([instance, size, offset]() mutable {
    instance->RT_SetData(instance->local_data_.Data, size, offset);
  });
}

void VulkanBuffer::RT_SetData(void* buffer, uint64_t size, uint64_t offset) {
  VulkanAllocator allocator(name_);
  uint8_t* pData = allocator.MapMemory<uint8_t>(memory_allocation_);
  memcpy(pData, (uint8_t*)buffer + offset, size);
  allocator.UnmapMemory(memory_allocation_);
}

VkDeviceAddress VulkanBuffer::Address() const {
  VkBufferDeviceAddressInfo info;
  info.buffer = buffer_;
  return vkGetBufferDeviceAddress(
      VulkanContext::GetCurrentDevice()->GetVulkanDevice(), &info);
}

void VulkanBuffer::SetName(std::string_view name) { name_ = name; }

void VulkanBuffer::RT_CreateBuffer(void* data, uint32_t size,
                                   VkBufferUsageFlags usage_flags) {
  auto device = VulkanContext::GetCurrentDevice();
  VulkanAllocator allocator(name_);

  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = descriptor_info_.range;
  bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VkBuffer staging_buffer;
  const VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(
      bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

  uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
  memcpy(destData, local_data_.Data, local_data_.Size);
  allocator.UnmapMemory(stagingBufferAllocation);

  VkBufferCreateInfo vertexBufferCreateInfo = {};
  vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  vertexBufferCreateInfo.size = descriptor_info_.range;
  vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_flags;
  memory_allocation_ = allocator.AllocateBuffer(
      vertexBufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, buffer_);

  const VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

  VkBufferCopy copy_region = {};
  copy_region.size = local_data_.Size;
  vkCmdCopyBuffer(copy_cmd, staging_buffer, buffer_, 1, &copy_region);

  device->FlushCommandBuffer(copy_cmd);

  allocator.DestroyBuffer(staging_buffer, stagingBufferAllocation);
}

void VulkanBuffer::RT_CreateBuffer(VkDeviceSize size,
                                   VkBufferUsageFlags usage) {
  auto device = VulkanContext::GetCurrentDevice();
  VulkanAllocator allocator(name_);

  VkBufferCreateInfo buffer_create_info = {};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.size = descriptor_info_.range;
  buffer_create_info.usage = usage;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  memory_allocation_ = allocator.AllocateBuffer(
      buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, buffer_);
  descriptor_info_.buffer = buffer_;
}

void VulkanBuffer::Alloc(VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags memoryTypeFlags) {}
}  // namespace base_engine

// @VulkanBuffer.h
// @brief
// @author ICE
// @date 2024/03/15
//
// @details

#pragma once

#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Buffer.h"
#include "Ref.h"
#include "VulkanDevice.h"

namespace base_engine {
class VulkanBuffer : public RefCounted {
 public:
  VulkanBuffer(const std::string& name, const void* data, uint32_t size,
               VkBufferUsageFlags usage_flags, bool RT = true);

  VulkanBuffer(const std::string& name, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_type,
               bool RT = true);
  ~VulkanBuffer();

  operator VkBuffer() const;

  VkDeviceSize size() const;

  const VkDescriptorBufferInfo& DescriptorInfo() const;

  void* Map();
  void Unmap();
  void SetData(void* buffer, uint64_t size, uint64_t offset = 0);
  void RT_SetData(void* buffer, uint64_t size, uint64_t offset = 0);
  VkDeviceAddress Address() const;

  void SetName(std::string_view name);

 private:
  void RT_CreateBuffer(void* data, uint32_t size,
                       VkBufferUsageFlags usage_flags);
  void RT_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

  void Alloc(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryTypeFlags);

  std::string name_;

  Buffer local_data_;

  VkBuffer buffer_ = VK_NULL_HANDLE;

  void* mapped_data_ = nullptr;

  VkDescriptorBufferInfo descriptor_info_;
  VmaAllocation memory_allocation_;
};

static VkDeviceAddress GetBufferDeviceAddress(VulkanDevice* Device,
                                              VkBuffer Buffer) {
  VkBufferDeviceAddressInfoKHR DeviceAddressInfo{};
  DeviceAddressInfo.buffer = Buffer;
  return vkGetBufferDeviceAddress(Device->GetVulkanDevice(),
                                  &DeviceAddressInfo);
}
static VkDeviceAddress GetBufferDeviceAddress(VkDevice Device,
                                              VkBuffer Buffer) {
  VkBufferDeviceAddressInfoKHR DeviceAddressInfo{};
  DeviceAddressInfo.buffer = Buffer;
  return vkGetBufferDeviceAddress(Device,
                                  &DeviceAddressInfo);
}
}  // namespace base_engine

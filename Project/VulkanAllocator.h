// @VulkanAllocator.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <vk_mem_alloc.h>

#include <string>

#include "GPUMemoryStats.h"
#include "Ref.h"
#include "VulkanDevice.h"

namespace base_engine {
class VulkanAllocator {
 public:
  VulkanAllocator() = default;
  VulkanAllocator(const std::string& tag);
  ~VulkanAllocator();

  VmaAllocation AllocateBuffer(const VkBufferCreateInfo& buffer_create_info,
                               VmaMemoryUsage usage, VkBuffer& out_buffer);
  VmaAllocation AllocateImage(VkImageCreateInfo image_create_info,
                              VmaMemoryUsage usage, VkImage& out_image,
                              VkDeviceSize* allocated_size = nullptr);
  static void Free(VmaAllocation allocation);
  void DestroyImage(VkImage image, VmaAllocation allocation);
  void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

  template <typename T>
  T* MapMemory(const VmaAllocation allocation) {
    T* mapped_memory;
    vmaMapMemory(VulkanAllocator::GetVMAAllocator(), allocation,
                 (void**)&mapped_memory);
    return mapped_memory;
  }

  void UnmapMemory(VmaAllocation allocation);

  static void DumpStats();
  static GPUMemoryStats GetStats();

  static void Init(Ref<VulkanDevice> device);
  static void Shutdown();

  static VmaAllocator& GetVMAAllocator();

 private:
  std::string tag_;
};
}  // namespace base_engine

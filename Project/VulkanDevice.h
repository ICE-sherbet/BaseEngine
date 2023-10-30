﻿// @VulkanDevice.h
// @brief
// @author ICE
// @date 2023/10/31
//
// @details
#pragma once
#include <vulkan/vulkan.h>

#include <map>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "Ref.h"

namespace base_engine {

class VulkanDevice;

class VulkanPhysicalDevice : public RefCounted {
 public:
  struct QueueFamilyIndices {
    int32_t graphics = -1;
  };

  VulkanPhysicalDevice();
  ~VulkanPhysicalDevice();

  void GetDeviceCreateInfo(VkDeviceCreateInfo* info) const;
  VkPhysicalDevice GetVulkanPhysicalDevice() const;
  void GetGraphicDeviceQueue(VkDevice device, VkQueue* queue) const;

  QueueFamilyIndices GetIndices() const { return queue_family_indices_; }

  static Ref<VulkanPhysicalDevice> Select();

 private:
  QueueFamilyIndices GetQueueFamilyIndices(int queue_flags) const;
  VkFormat FindDepthFormat() const;

  VkPhysicalDevice physical_device_ = nullptr;
  VkPhysicalDeviceProperties properties_;
  VkPhysicalDeviceFeatures features_;
  VkPhysicalDeviceMemoryProperties memory_properties_;

  VkFormat depth_format_ = VK_FORMAT_UNDEFINED;

  QueueFamilyIndices queue_family_indices_;
  std::vector<VkQueueFamilyProperties> queue_family_properties_;
  std::unordered_set<std::string> supported_extensions_;
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos_;
};

class VulkanCommandPool : public RefCounted {
 public:
  VulkanCommandPool();

  virtual ~VulkanCommandPool();

  VkCommandBuffer AllocateCommandBuffer(bool begin, bool compute = false) const;
  void FlushCommandBuffer(VkCommandBuffer command_buffer) const;
  void FlushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue) const;

  VkCommandPool GetGraphicsCommandPool() const {
    return graphics_command_pool_;
  }
  VkCommandPool GetComputeCommandPool() const { return compute_command_pool_; }

 private:
  VkCommandPool graphics_command_pool_ = 0;
  VkCommandPool compute_command_pool_ = 0;
};

class VulkanDevice : public RefCounted {
 public:
  VulkanDevice(const Ref<VulkanPhysicalDevice>& physical_device,
               VkPhysicalDeviceFeatures enabled_features);

  VkQueue GetGraphicsQueue() const { return graphics_queue_; }

  void CreatePool(const VkCommandPoolCreateInfo& pool_info,
                  VkCommandPool* pool) const;
  void DestroyPool(VkCommandPool pool) const;

  void AllocateCommandBuffers(const VkCommandBufferAllocateInfo& info,
                              VkCommandBuffer* buffers) const;

  uint32_t GetGraphicsIndex() const;

  VkDevice GetVulkanDevice() const { return logical_device_; }

 private:
  VkDevice logical_device_ = nullptr;
  Ref<VulkanPhysicalDevice> physical_device_;
  VkPhysicalDeviceFeatures enabled_features_;

  VkQueue graphics_queue_;
  VkQueue compute_queue_;

  std::map<std::thread::id, Ref<VulkanCommandPool>> command_pools_;
};
}  // namespace base_engine
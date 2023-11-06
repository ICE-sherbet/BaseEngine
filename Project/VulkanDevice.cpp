#include "VulkanDevice.h"

#include "Assert.h"
#include "VulkanContext.h"
#include "VulkanUtilities.h"

namespace base_engine {
VulkanPhysicalDevice::VulkanPhysicalDevice() {
  const auto instance = VulkanContext::GetVkInstance();
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  BE_CORE_ASSERT(device_count > 0, "GPU not support Vulkan");

  std::vector<VkPhysicalDevice> physical_devices(device_count);
  vulkan::VulkanCheckResult(vkEnumeratePhysicalDevices(
      instance, &device_count, physical_devices.data()));

  VkPhysicalDevice selected_physical_device = nullptr;
  for (const auto physical_device : physical_devices) {
    vkGetPhysicalDeviceProperties(physical_device, &properties_);
    if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      selected_physical_device = physical_device;
      break;
    }
  }
  BE_CORE_ASSERT(selected_physical_device != nullptr,
                 "Could not find discrete GPU.");

  physical_device_ = selected_physical_device;
  vkGetPhysicalDeviceFeatures(physical_device_, &features_);
  vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);

  // Get queue family properties
  {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_,
                                             &queue_family_count, nullptr);
    BE_CORE_ASSERT(queue_family_count > 0, "Could not get queue family count.");
    queue_family_properties_.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device_, &queue_family_count, queue_family_properties_.data());
  }

  {
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device_, nullptr,
                                         &extension_count, nullptr);
    if (extension_count > 0) {
      if (std::vector<VkExtensionProperties> extensions(extension_count);
          vkEnumerateDeviceExtensionProperties(
              physical_device_, nullptr, &extension_count,
              &extensions.front()) == VK_SUCCESS) {
        for (const auto& [extensionName, specVersion] : extensions) {
          supported_extensions_.emplace(extensionName);
        }
      }
    }
  }

  constexpr int kQueueFlags = VK_QUEUE_GRAPHICS_BIT;

  queue_family_indices_ = GetQueueFamilyIndices(kQueueFlags);
  if (kQueueFlags & VK_QUEUE_GRAPHICS_BIT) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_indices_.graphics;
    queue_create_info.queueCount = 1;
    constexpr float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos_.emplace_back(queue_create_info);
  }

  depth_format_ = FindDepthFormat();
}

VulkanPhysicalDevice::~VulkanPhysicalDevice() = default;

void VulkanPhysicalDevice::GetDeviceCreateInfo(VkDeviceCreateInfo* info) const {
  if (info == nullptr) return;

  VkDeviceCreateInfo device_create_info = {};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = queue_create_infos_.size();

  device_create_info.pQueueCreateInfos = queue_create_infos_.data();

  *info = device_create_info;
}

VkPhysicalDevice VulkanPhysicalDevice::GetVulkanPhysicalDevice() const {
  return physical_device_;
}

void VulkanPhysicalDevice::GetGraphicDeviceQueue(VkDevice device,
                                                 VkQueue* queue) const {
  vkGetDeviceQueue(device, queue_family_indices_.graphics, 0, queue);
}

Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Select() {
  return Ref<VulkanPhysicalDevice>::Create();
}

VulkanPhysicalDevice::QueueFamilyIndices
VulkanPhysicalDevice::GetQueueFamilyIndices(const int queue_flags) const {
  QueueFamilyIndices indices;
  for (size_t i = 0; i < queue_family_properties_.size(); i++) {
    if (queue_flags & VK_QUEUE_GRAPHICS_BIT) {
      if (queue_family_properties_[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        indices.graphics = i;
    }
  }

  return indices;
}

VkFormat VulkanPhysicalDevice::FindDepthFormat() const {
  for (const std::vector depth_formats = {VK_FORMAT_D32_SFLOAT_S8_UINT,
                                          VK_FORMAT_D32_SFLOAT,
                                          VK_FORMAT_D24_UNORM_S8_UINT,
                                          VK_FORMAT_D16_UNORM_S8_UINT,
                                          VK_FORMAT_D16_UNORM};
       const auto& format : depth_formats) {
    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(physical_device_, format,
                                        &format_props);
    if (format_props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      return format;
  }

  return VK_FORMAT_UNDEFINED;
}

VulkanCommandPool::VulkanCommandPool() {
  auto device = VulkanContext::GetCurrentDevice();

  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = device->GetGraphicsIndex();
  cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  device->CreatePool(cmdPoolInfo, &graphics_command_pool_);
}

VulkanCommandPool::~VulkanCommandPool() {
  auto device = VulkanContext::GetCurrentDevice();
  device->DestroyPool(graphics_command_pool_);
}

VkCommandBuffer VulkanCommandPool::AllocateCommandBuffer(
    const bool begin, const bool compute) const {
  VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
  cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufAllocateInfo.commandPool = graphics_command_pool_;
  cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufAllocateInfo.commandBufferCount = 1;

  VkCommandBuffer cmd_buffer;
  VulkanContext::GetCurrentDevice()->AllocateCommandBuffers(cmdBufAllocateInfo,
                                                            &cmd_buffer);

  if (begin) {
    VkCommandBufferBeginInfo cmd_buf_info{};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (compute) {
      cmd_buf_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }
    vkBeginCommandBuffer(cmd_buffer, &cmd_buf_info);
  }

  return cmd_buffer;
}

void VulkanCommandPool::FlushCommandBuffer(
    const VkCommandBuffer command_buffer) const {
  FlushCommandBuffer(command_buffer,
                     VulkanContext::GetCurrentDevice()->GetGraphicsQueue());
}

void VulkanCommandPool::FlushCommandBuffer(const VkCommandBuffer command_buffer,
                                           const VkQueue queue) const {
  auto device = VulkanContext::GetCurrentDevice();
  const auto vulkan_device = device->GetVulkanDevice();

  constexpr uint64_t default_fence_timeout = 100000000000;

  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_create_info.flags = 0;
  VkFence fence;

  vkCreateFence(vulkan_device, &fence_create_info, nullptr, &fence);

  {
    static std::mutex submission_lock;
    std::scoped_lock lock(submission_lock);
    vkQueueSubmit(queue, 1, &submit_info, fence);
  }

  vkWaitForFences(vulkan_device, 1, &fence, VK_TRUE, default_fence_timeout);

  vkDestroyFence(vulkan_device, fence, nullptr);
  vkFreeCommandBuffers(vulkan_device, graphics_command_pool_, 1,
                       &command_buffer);
}
VulkanDevice::VulkanDevice(const Ref<VulkanPhysicalDevice>& physical_device,
                           VkPhysicalDeviceFeatures enabled_features)
    : physical_device_(physical_device), enabled_features_(enabled_features) {
  std::vector<const char*> device_extensions;

  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceCreateInfo device_create_info{};
  physical_device_->GetDeviceCreateInfo(&device_create_info);
  device_create_info.pEnabledFeatures = &enabled_features;

  if (!device_extensions.empty()) {
    device_create_info.enabledExtensionCount = device_extensions.size();
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
  }

  vkCreateDevice(physical_device_->GetVulkanPhysicalDevice(),
                 &device_create_info, nullptr, &logical_device_);

  physical_device_->GetGraphicDeviceQueue(logical_device_, &graphics_queue_);
}

void VulkanDevice::CreatePool(const VkCommandPoolCreateInfo& pool_info,
                              VkCommandPool* pool) const {
  vkCreateCommandPool(logical_device_, &pool_info, nullptr, pool);
}

void VulkanDevice::DestroyPool(const VkCommandPool pool) const {
  vkDestroyCommandPool(logical_device_, pool, nullptr);
}

void VulkanDevice::AllocateCommandBuffers(
    const VkCommandBufferAllocateInfo& info, VkCommandBuffer* buffers) const {
  vkAllocateCommandBuffers(logical_device_, &info, buffers);
}

uint32_t VulkanDevice::GetGraphicsIndex() const {
  return physical_device_->GetIndices().graphics;
}

VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer(
    const char* debug_name) {
  VkCommandBuffer cmd_buffer;

  VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
  cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufAllocateInfo.commandPool =
      GetOrCreateThreadLocalCommandPool()->GetGraphicsCommandPool();
  cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  cmdBufAllocateInfo.commandBufferCount = 1;

  vkAllocateCommandBuffers(logical_device_, &cmdBufAllocateInfo, &cmd_buffer);
  return cmd_buffer;
}

VkCommandBuffer VulkanDevice::GetCommandBuffer(const bool begin) {
  return GetOrCreateThreadLocalCommandPool()->AllocateCommandBuffer(begin,
                                                                    false);
}

void VulkanDevice::FlushCommandBuffer(const VkCommandBuffer command_buffer) {
  GetThreadLocalCommandPool()->FlushCommandBuffer(command_buffer);
}

Ref<VulkanCommandPool> VulkanDevice::GetThreadLocalCommandPool() {
  const auto thread_id = std::this_thread::get_id();
  const auto command_pool_it = command_pools_.find(thread_id);
  BE_CORE_ASSERT(command_pool_it != command_pools_.end(),
                 "Command pool not found for thread.");
  return command_pool_it->second;
}

Ref<VulkanCommandPool> VulkanDevice::GetOrCreateThreadLocalCommandPool() {
  const auto thread_id = std::this_thread::get_id();
  const auto command_pool_it = command_pools_.find(thread_id);
  if (command_pool_it != command_pools_.end()) return command_pool_it->second;

  Ref<VulkanCommandPool> command_pool = Ref<VulkanCommandPool>::Create();
  command_pools_[thread_id] = command_pool;
  return command_pool;
}
}  // namespace base_engine

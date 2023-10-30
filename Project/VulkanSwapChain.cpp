#include "VulkanSwapChain.h"

namespace base_engine {
void VulkanSwapChain::Init(VkInstance instance,
                           const Ref<VulkanDevice>& device) {
  m_Instance = instance;
  m_Device = device;
}

void VulkanSwapChain::InitSurface(GLFWwindow* windowHandle) {
  VkPhysicalDevice physicalDevice = m_Device->GetVulkanPhysicalDevice();
  glfwCreateWindowSurface(m_Instance, windowHandle, nullptr, &m_Surface);

  uint32_t surfaceFormatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface,
                                       &surfaceFormatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface,
                                       &surfaceFormatCount, formats.data());

  // 検索して一致するフォーマットを見つける.
  for (const auto& f : formats) {
    if (f.format == VK_FORMAT_B8G8R8A8_UNORM) {
      m_ColorFormat = f.format;
      m_ColorSpace = f.colorSpace;
    }
  }

  VkSurfaceCapabilitiesKHR surfCaps;

  uint32_t queueCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
  BE_CORE_ASSERT(queueCount >= 1);

  std::vector<VkQueueFamilyProperties> queueProps(queueCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount,
                                           queueProps.data());

  std::vector<VkBool32> supportsPresent(queueCount);

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface,
                                            &surfCaps);

  for (uint32_t i = 0; i < queueCount; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface,
                                         &supportsPresent[i]);
  }
  uint32_t graphicsQueueNodeIndex = UINT32_MAX;
  uint32_t presentQueueNodeIndex = UINT32_MAX;
  for (uint32_t i = 0; i < queueCount; i++) {
    if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      if (graphicsQueueNodeIndex == UINT32_MAX) {
        graphicsQueueNodeIndex = i;
      }

      if (supportsPresent[i] == VK_TRUE) {
        graphicsQueueNodeIndex = i;
        presentQueueNodeIndex = i;
        break;
      }
    }
  }
  if (presentQueueNodeIndex == UINT32_MAX) {
    for (uint32_t i = 0; i < queueCount; ++i) {
      if (supportsPresent[i] == VK_TRUE) {
        presentQueueNodeIndex = i;
        break;
      }
    }
  }

  m_QueueNodeIndex = graphicsQueueNodeIndex;
}
}  // namespace base_engine

#include "VulkanContext.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Assert.h"

namespace base_engine {
void VulkanContext::Init() {
  BE_CORE_ASSERT(glfwVulkanSupported(), "GLFW not support Vulkan");

  InitInstance();

  physics_device_ = VulkanPhysicalDevice::Select();
  VkPhysicalDeviceFeatures enabled_features = {};
  enabled_features.samplerAnisotropy = true;
  enabled_features.wideLines = true;
  enabled_features.fillModeNonSolid = true;
  enabled_features.independentBlend = true;
  enabled_features.pipelineStatisticsQuery = true;
  device_ = Ref<VulkanDevice>::Create(physics_device_, enabled_features);

  VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
  pipeline_cache_create_info.sType =
      VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  vkCreatePipelineCache(device_->GetVulkanDevice(), &pipeline_cache_create_info,
                        nullptr, &pipeline_cache_);
}

VulkanContext::~VulkanContext() {
  vkDestroyInstance(instance_, nullptr);
  instance_ = nullptr;
}

void VulkanContext::InitInstance() {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "Base Engine";
  app_info.pEngineName = "Base Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_2;

#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
  std::vector instance_extensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                     VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
  instance_extensions.push_back(
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // Very little performance hit, can
                                           // be used in Release.
  instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  instance_extensions.push_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

  VkInstanceCreateInfo instance_create_info{};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &app_info;
  instance_create_info.pNext = nullptr;
  instance_create_info.enabledExtensionCount =
      (uint32_t)instance_extensions.size();
  instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
  auto err = vkCreateInstance(&instance_create_info, nullptr, &instance_);
  if (VK_SUCCESS != err) {
    BE_CORE_ERROR("Vulkan error.");
    std::exit(-1);
  }
}
}  // namespace base_engine

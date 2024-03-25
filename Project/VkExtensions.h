// @VkExtensions.h
// @brief
// @author ICE
// @date 2024/03/19
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>
void load_VK_EXTENSIONS(VkInstance instance,
                        PFN_vkGetInstanceProcAddr getInstanceProcAddr,
                        VkDevice device,
                        PFN_vkGetDeviceProcAddr getDeviceProcAddr);

// @VulkanUtilities.h
// @brief
// @author ICE
// @date 2023/10/31
// 
// @details

#pragma once
#include <vulkan/vulkan.h>

#include "Log.h"

namespace base_engine::vulkan
{
inline void VulkanCheckResult(VkResult result) {
  if (result != VK_SUCCESS) {
    //TODO エラー処理
    BE_CORE_ERROR("Vulkan error.");
  }
}
}

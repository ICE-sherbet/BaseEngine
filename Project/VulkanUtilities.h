// @VulkanUtilities.h
// @brief
// @author ICE
// @date 2023/10/31
//
// @details

#pragma once
#include <vulkan/vulkan.h>

#include "Log.h"

namespace base_engine::vulkan {
inline void VulkanCheckResult(VkResult result) {
  if (result != VK_SUCCESS) {
    // TODO エラー処理
    BE_CORE_ERROR("Vulkan error.");
  }
}

static void SetDebugUtilsObjectName(const VkDevice device,
                                    const VkObjectType objectType,
                                    const std::string& name,
                                    const uint64_t handle) {
  VkDebugUtilsObjectNameInfoEXT nameInfo;
  nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
  nameInfo.objectType = objectType;
  nameInfo.pObjectName = name.c_str();
  nameInfo.objectHandle = handle;
  nameInfo.pNext = nullptr;

  vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
}

void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image,
                              VkAccessFlags srcAccessMask,
                              VkAccessFlags dstAccessMask,
                              VkImageLayout oldImageLayout,
                              VkImageLayout newImageLayout,
                              VkPipelineStageFlags srcStageMask,
                              VkPipelineStageFlags dstStageMask,
                              VkImageSubresourceRange subresourceRange) {
  VkImageMemoryBarrier imageMemoryBarrier{};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  imageMemoryBarrier.srcAccessMask = srcAccessMask;
  imageMemoryBarrier.dstAccessMask = dstAccessMask;
  imageMemoryBarrier.oldLayout = oldImageLayout;
  imageMemoryBarrier.newLayout = newImageLayout;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0,
                       nullptr, 1, &imageMemoryBarrier);
}
}  // namespace base_engine::vulkan

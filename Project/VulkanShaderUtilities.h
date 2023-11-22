// @VulkanShaderUtilities.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <shaderc/shaderc.h>
#include <vulkan/vulkan.h>

#include <string_view>

namespace base_engine::shader_utils {
static VkShaderStageFlagBits ShaderTypeFromString(const std::string_view type) {
  if (type == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
  if (type == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
  if (type == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;

  return VK_SHADER_STAGE_ALL;
}
static const char* ShaderStageToString(const VkShaderStageFlagBits stage) {
  switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
      return "vert";
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      return "frag";
    case VK_SHADER_STAGE_COMPUTE_BIT:
      return "comp";
  }
  return "UNKNOWN";
}

static shaderc_shader_kind ShaderStageToShaderC(
    const VkShaderStageFlagBits stage) {
  switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT:
      return shaderc_vertex_shader;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      return shaderc_fragment_shader;
    case VK_SHADER_STAGE_COMPUTE_BIT:
      return shaderc_compute_shader;
  }
  return {};
}

static std::string_view VKStageToShaderMacro(const VkShaderStageFlagBits stage) {
  if (stage == VK_SHADER_STAGE_VERTEX_BIT) return "__VERTEX_STAGE__";
  if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return "__FRAGMENT_STAGE__";
  if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return "__COMPUTE_STAGE__";
  return "";
}
};  // namespace base_engine::shader_utils

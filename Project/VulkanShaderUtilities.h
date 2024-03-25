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

#include "Assert.h"

namespace base_engine::shader_utils {


inline static std::string_view StageToShaderMacro(
    const std::string_view stage) {
  if (stage == "vert") return "__VERTEX_STAGE__";
  if (stage == "frag") return "__FRAGMENT_STAGE__";
  if (stage == "comp") return "__COMPUTE_STAGE__";
  return "";
}

static VkShaderStageFlagBits ShaderTypeFromString(const std::string_view type) {
  if (type == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
  if (type == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
  if (type == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;
  if (type == "rgen") return VK_SHADER_STAGE_RAYGEN_BIT_NV;
  if (type == "rmiss") return VK_SHADER_STAGE_MISS_BIT_NV;
  if (type == "rchit") return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

  return VK_SHADER_STAGE_ALL;
}

inline static VkShaderStageFlagBits StageToVKShaderStage(
    const std::string_view stage) {
  if (stage == "vert") return VK_SHADER_STAGE_VERTEX_BIT;
  if (stage == "frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
  if (stage == "comp") return VK_SHADER_STAGE_COMPUTE_BIT;
  if (stage == "rgen") return VK_SHADER_STAGE_RAYGEN_BIT_NV;
  if (stage == "rmiss") return VK_SHADER_STAGE_MISS_BIT_NV;
  if (stage == "rchit") return VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

  BE_CORE_VERIFY(false, "Unknown shader stage.");
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
    case VK_SHADER_STAGE_RAYGEN_BIT_NV:
      return "rgen";
    case VK_SHADER_STAGE_MISS_BIT_NV:
			return "rmiss";
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
			return "rchit";
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
    case VK_SHADER_STAGE_RAYGEN_BIT_NV:
      return shaderc_raygen_shader;
    case VK_SHADER_STAGE_MISS_BIT_NV:
			return shaderc_miss_shader;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
      return shaderc_closesthit_shader;
  }
  return {};
}

static std::string_view VKStageToShaderMacro(
    const VkShaderStageFlagBits stage) {
  if (stage == VK_SHADER_STAGE_VERTEX_BIT) return "__VERTEX_STAGE__";
  if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) return "__FRAGMENT_STAGE__";
  if (stage == VK_SHADER_STAGE_COMPUTE_BIT) return "__COMPUTE_STAGE__";
  if (stage == VK_SHADER_STAGE_RAYGEN_BIT_NV) return "__RAYGEN_STAGE__";
  if (stage == VK_SHADER_STAGE_MISS_BIT_NV) return "__MISS_STAGE__";
  if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV) return "__CLOSET_STAGE__";
  return "";
}
};  // namespace base_engine::shader_utils

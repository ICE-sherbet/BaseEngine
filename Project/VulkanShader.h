// @VulkanShader.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <map>
#include <spirv_cross/spirv_cross.hpp>

#include "Shader.h"

namespace base_engine {
using ShaderDate = std::map<VkShaderStageFlagBits, std::vector<uint32_t>>;

class VulkanShader : public Shader {
 public:
  VulkanShader() = default;
  VulkanShader(const std::string& path, bool forceCompile,
               bool disableOptimization);
  void Reload(bool forceCompile) override;
  void RT_Reload(bool forceCompile) override;
  void Release();

  void LoadAndCreateShaders(const ShaderDate& shaderData);
  void CreateDescriptors();

 private:
  std::filesystem::path m_AssetPath;
  std::string m_Name;
  bool m_DisableOptimization = false;

  std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageCreateInfos;
  std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_ShaderData;

  std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
  VkDescriptorSet m_DescriptorSet;
  friend class VulkanShaderCompiler;
};
}  // namespace base_engine

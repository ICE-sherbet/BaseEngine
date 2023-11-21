// @VulkanShaderCompiler.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once

#include <vulkan/vulkan_core.h>

#include <map>

#include "VulkanReflectionData.h"
#include "VulkanShader.h"

namespace base_engine {

class VulkanShaderCompiler : public RefCounted {
 public:
  VulkanShaderCompiler(const std::filesystem::path& shaderSourcePath,
                       bool disableOptimization);

  bool Reload(bool forceCompile = false);
  static Ref<VulkanShader> Compile(
      const std::filesystem::path& shaderSourcePath, bool forceCompile = false,
      bool disableOptimization = false);

  static bool TryRecompile(Ref<VulkanShader> shader);
  void ReflectAllShaderStages(const ShaderDate& shaderData);
  void Reflect(VkShaderStageFlagBits shaderStage,
               const std::vector<uint32_t>& shaderData);

  const ShaderDate& GetSPIRVData() const { return m_SPIRVData; }

 private:
  std::map<VkShaderStageFlagBits, std::string> PreProcess(
      const std::string& source);

  void ClearReflectionData();

  friend class VulkanShader;

  ReflectionData m_ReflectionData;

  std::filesystem::path m_ShaderSourcePath;
  bool m_DisableOptimization = false;
  std::map<VkShaderStageFlagBits, std::string> m_ShaderSource;
  ShaderDate m_SPIRVDebugData, m_SPIRVData;
};
}  // namespace base_engine

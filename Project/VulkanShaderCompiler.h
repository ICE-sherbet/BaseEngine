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
struct StageData {
  uint32_t HashValue = 0;
};
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

  const ShaderDate& GetSPIRVData() const { return spirv_data_; }

 private:
  bool Compile(std::vector<uint32_t>& outputBinary,
               const VkShaderStageFlagBits stage) const;

  bool CompileOrGetVulkanBinaries(ShaderDate& outputBinary);
  bool CompileOrGetVulkanBinary(VkShaderStageFlagBits stage,
                                std::vector<uint32_t>& outputBinary);

  std::map<VkShaderStageFlagBits, std::string> PreProcess(
      const std::string& source);

  void ClearReflectionData();

  friend class VulkanShader;

  ReflectionData reflection_data_;

  std::filesystem::path shader_source_path_;
  bool disable_optimization_ = false;
  std::map<VkShaderStageFlagBits, std::string> shader_source_;
  ShaderDate spirv_debug_data_, spirv_data_;
};
}  // namespace base_engine

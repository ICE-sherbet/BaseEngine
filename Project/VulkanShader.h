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
#include "VulkanReflectionData.h"

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

  const std::unordered_map<std::string, ShaderBuffer>& GetShaderBuffers()
      const {
    return reflection_data_.ConstantBuffers;
  }

  const std::vector<shader::ShaderDescriptorSet>& GetShaderDescriptorSets()
      const {
    return reflection_data_.ShaderDescriptorSets;
  }

  std::vector<VkDescriptorSetLayout> GetAllDescriptorSetLayouts();
  const std::vector<shader::PushConstantRange>& GetPushConstantRanges() const {
    return reflection_data_.PushConstantRanges;
  }

  const std::vector<VkPipelineShaderStageCreateInfo>&
  GetPipelineShaderStageCreateInfos() const {
    return pipeline_shader_stage_create_infos_;
  }

  VkDescriptorSetLayout GetDescriptorSetLayout(const uint32_t set) const {
    return descriptor_set_layouts_.at(set);
  }
  const std::string& GetName() const override { return name_; }

 private:
  std::filesystem::path asset_path_;
  std::string name_;
  bool disable_optimization_ = false;

  std::vector<VkPipelineShaderStageCreateInfo>
      pipeline_shader_stage_create_infos_;
  std::map<VkShaderStageFlagBits, std::vector<uint32_t>> shader_data_;

  std::vector<VkDescriptorSetLayout> descriptor_set_layouts_;
  std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> type_counts_;
  VkDescriptorSet descriptor_set_;

  ReflectionData reflection_data_;
  friend class VulkanShaderCompiler;
  void SetReflectionData(const ReflectionData& reflection_data);
};
}  // namespace base_engine

// @VulkanReflectionData.h
// @brief
// @author ICE
// @date 2023/11/20
// 
// @details

#pragma once
#include "Shader.h"
#include "VulkanShaderResource.h"

namespace base_engine
{
struct ReflectionData {
  std::vector<shader::ShaderDescriptorSet> ShaderDescriptorSets;
  std::unordered_map<std::string, ShaderBuffer> ConstantBuffers;
  std::vector<shader::PushConstantRange> PushConstantRanges;
};
}

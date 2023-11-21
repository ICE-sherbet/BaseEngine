#include "VulkanShader.h"

#include "Log.h"
#include "RendererApi.h"
#include "VulkanContext.h"
#include "VulkanShaderCompiler.h"

namespace base_engine {
VulkanShader::VulkanShader(const std::string& path, bool forceCompile,
                           bool disableOptimization) {
  size_t found = path.find_last_of("/\\");
  m_Name = found != std::string::npos ? path.substr(found + 1) : path;
  found = m_Name.find_last_of('.');
  m_Name = found != std::string::npos ? m_Name.substr(0, found) : m_Name;

  Reload(forceCompile);
}

void VulkanShader::Reload(bool forceCompile) {
  Renderer::Submit([instance = Ref(this), forceCompile]() mutable {
    instance->RT_Reload(forceCompile);
  });
}

void VulkanShader::RT_Reload(bool forceCompile) {
  if (!VulkanShaderCompiler::TryRecompile(this)) {
    BE_CORE_FATAL("Failed to recompile shader!");
  }
}

void VulkanShader::Release() {}

void VulkanShader::LoadAndCreateShaders(
    const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData) {
  m_ShaderData = shaderData;

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  m_PipelineShaderStageCreateInfos.clear();
  std::string moduleName;
  for (auto [stage, data] : shaderData) {
    VkShaderModuleCreateInfo moduleCreateInfo{};

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = data.data();

    VkShaderModule shaderModule;

    vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule);

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = shaderModule;
    shaderStage.pName = "main";
    m_PipelineShaderStageCreateInfos.emplace_back(shaderStage);
  }
}

void VulkanShader::CreateDescriptors() {
  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

}
}  // namespace base_engine

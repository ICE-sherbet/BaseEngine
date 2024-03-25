#include "Shader.h"

#include "RendererApi.h"
#include "VulkanShader.h"
#include "VulkanShaderCompiler.h"

namespace base_engine {
ShaderUniform::ShaderUniform(const std::string& name,
                             const ShaderUniformType type, const uint32_t size,
                             const uint32_t offset) {
  name_ = name;
  type_ = type;
  size_ = size;
  offset_ = offset;
}

Ref<Shader> Shader::Create(const std::string& filepath, bool forceCompile,
                           bool disableOptimization) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanShader>::Create(filepath, forceCompile,
                                       disableOptimization);
  }
}

ShaderRegistry::ShaderRegistry() = default;

ShaderRegistry::~ShaderRegistry() = default;

void ShaderRegistry::Add(const Ref<Shader>& shader) {
  auto& name = shader->GetName();
  m_Shaders[name] = shader;
}

void ShaderRegistry::Load(std::string_view path, bool forceCompile,
                          bool disableOptimization) {
  Ref<Shader> shader;
  VulkanShaderCompiler::Compile(path, forceCompile, disableOptimization);

  auto& name = shader->GetName();
  m_Shaders[name] = shader;
}

void ShaderRegistry::Load(std::string_view name, const std::string& path) {
  m_Shaders[std::string(name)] = Shader::Create(path);
}

const Ref<Shader>& ShaderRegistry::Get(const std::string& name) const {
  return m_Shaders.at(name);
}
}  // namespace base_engine
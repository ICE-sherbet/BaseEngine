#include "Shader.h"

#include "RendererApi.h"
#include "VulkanShader.h"

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
}  // namespace base_engine
#include "VulkanMaterial.h"

#include <ranges>

#include "RendererApi.h"

namespace base_engine {
VulkanMaterial::VulkanMaterial(const Ref<Shader>& shader,
                               std::string name)
    : shader_(shader), name_(std::move(name)) {
  Init();
}

VulkanMaterial::VulkanMaterial(Ref<Material> material,
                               const std::string& name) {
  shader_ = material->GetShader().As<VulkanShader>();
  name_ = name;
  Init();
}

void VulkanMaterial::Set(const std::string& name, const float value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const int value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const uint32_t value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const bool value) {
  SetBuffer<int>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec2& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec3& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec4& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec2& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec3& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec4& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::mat3& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::mat4& value) {
  SetBuffer(name, value);
}

void VulkanMaterial::Set(const std::string& name,
                         const Ref<RendererTexture2D>& texture) {
  descriptor_set_manager_.SetInput(name, texture);
}

void VulkanMaterial::Set(const std::string& name,
                         const Ref<RendererTexture2D>& texture,
                         uint32_t arrayIndex) {
  descriptor_set_manager_.SetInput(name, texture, arrayIndex);
}

float& VulkanMaterial::GetFloat(const std::string& name) {
  return GetBuffer<float>(name);
}

int32_t& VulkanMaterial::GetInt(const std::string& name) {
  return GetBuffer<int32_t>(name);
}

uint32_t& VulkanMaterial::GetUInt(const std::string& name) {
  return GetBuffer<uint32_t>(name);
}

bool& VulkanMaterial::GetBool(const std::string& name) {
  return GetBuffer<bool>(name);
}

glm::vec2& VulkanMaterial::GetVector2(const std::string& name) {
  return GetBuffer<glm::vec2>(name);
}

glm::vec3& VulkanMaterial::GetVector3(const std::string& name) {
  return GetBuffer<glm::vec3>(name);
}

glm::vec4& VulkanMaterial::GetVector4(const std::string& name) {
  return GetBuffer<glm::vec4>(name);
}

glm::mat3& VulkanMaterial::GetMatrix3(const std::string& name) {
  return GetBuffer<glm::mat3>(name);
}

glm::mat4& VulkanMaterial::GetMatrix4(const std::string& name) {
  return GetBuffer<glm::mat4>(name);
}

Ref<RendererTexture2D> VulkanMaterial::GetTexture2D(const std::string& name) {
  return descriptor_set_manager_.GetInput<RendererTexture2D>(name);
}

Ref<RendererTexture2D> VulkanMaterial::TryGetTexture2D(
    const std::string& name) {
  return descriptor_set_manager_.GetInput<RendererTexture2D>(name);
}

VkDescriptorSet VulkanMaterial::GetDescriptorSet(uint32_t index) {
  if (descriptor_set_manager_.GetFirstSetIndex() ==
      (std::numeric_limits<uint32_t>::max)())
    return VK_NULL_HANDLE;

  descriptor_set_manager_.InvalidateAndUpdate();

  return descriptor_set_manager_.GetDescriptorSets(index)[0];
}

Buffer VulkanMaterial::GetUniformStorageBuffer() const {
  return uniform_storage_buffer_;
}

const ShaderUniform* VulkanMaterial::FindUniformDeclaration(
    const std::string& name) {
  const auto& shader_buffers = shader_->GetShaderBuffers();

  BE_CORE_ASSERT(shader_buffers.size() <= 1,
                 "We currently only support ONE material buffer!");

  if (!shader_buffers.empty()) {
    const auto& [Name, Size, Uniforms] = shader_buffers.begin()->second;
    if (!Uniforms.contains(name)) return nullptr;
    return &Uniforms.at(name);
  }
  return nullptr;
}

void VulkanMaterial::Init() {
  AllocateStorage();

  DescriptorSetManagerSpecification dmSpec;
  dmSpec.DebugName =
      name_.empty() ? fmt::format("{} (kMaterial)", shader_->GetName()) : name_;
  dmSpec.Shader = shader_.As<VulkanShader>();
  dmSpec.StartSet = 0;
  dmSpec.EndSet = 0;
  dmSpec.DefaultResources = true;
  descriptor_set_manager_ = DescriptorSetManager(dmSpec);

  for (const auto& [name, decl] : descriptor_set_manager_.InputDeclarations) {
    switch (decl.Type) {
      case RenderPassInputType::ImageSampler1D:
      case RenderPassInputType::ImageSampler2D: {
        for (uint32_t i = 0; i < decl.Count; i++)
          descriptor_set_manager_.SetInput(name, Renderer::GetWhiteTexture(),
                                           i);

        break;
      }
    }
  }

  BE_CORE_VERIFY(descriptor_set_manager_.Validate());
  descriptor_set_manager_.Bake();
}

void VulkanMaterial::AllocateStorage() {
  if (const auto& shader_buffers = shader_->GetShaderBuffers();
      !shader_buffers.empty()) {
    uint32_t size = 0;
    for (const auto& [Name, Size, Uniforms] :
         shader_buffers | std::views::values)
      size += Size;

    uniform_storage_buffer_.Allocate(size);
    uniform_storage_buffer_.ZeroInitialize();
  }
}
}  // namespace base_engine

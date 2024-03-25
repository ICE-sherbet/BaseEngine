// @VulkanMaterial.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "Buffer.h"
#include "DescriptorSetManager.h"
#include "Material.h"
#include "VulkanShader.h"

namespace base_engine {
class VulkanMaterial : public Material {
 public:
  VulkanMaterial() = default;
  explicit VulkanMaterial(const Ref<Shader>& shader,
                          std::string name = "");
  explicit VulkanMaterial(Ref<Material> material, const std::string& name = "");

  virtual void Set(const std::string& name, float value) override;
  virtual void Set(const std::string& name, int value) override;
  virtual void Set(const std::string& name, uint32_t value) override;
  virtual void Set(const std::string& name, bool value) override;
  virtual void Set(const std::string& name, const glm::ivec2& value) override;
  virtual void Set(const std::string& name, const glm::ivec3& value) override;
  virtual void Set(const std::string& name, const glm::ivec4& value) override;
  virtual void Set(const std::string& name, const glm::vec2& value) override;
  virtual void Set(const std::string& name, const glm::vec3& value) override;
  virtual void Set(const std::string& name, const glm::vec4& value) override;
  virtual void Set(const std::string& name, const glm::mat3& value) override;
  virtual void Set(const std::string& name, const glm::mat4& value) override;

  void Set(const std::string& name,
           const Ref<RendererTexture2D>& texture) override;
  void Set(const std::string& name, const Ref<RendererTexture2D>& texture,
           uint32_t arrayIndex) override;

  virtual float& GetFloat(const std::string& name) override;
  virtual int32_t& GetInt(const std::string& name) override;
  virtual uint32_t& GetUInt(const std::string& name) override;
  virtual bool& GetBool(const std::string& name) override;
  virtual glm::vec2& GetVector2(const std::string& name) override;
  virtual glm::vec3& GetVector3(const std::string& name) override;
  virtual glm::vec4& GetVector4(const std::string& name) override;
  virtual glm::mat3& GetMatrix3(const std::string& name) override;
  virtual glm::mat4& GetMatrix4(const std::string& name) override;

  virtual Ref<RendererTexture2D> GetTexture2D(const std::string& name) override;

  virtual Ref<RendererTexture2D> TryGetTexture2D(
      const std::string& name) override;

  virtual uint32_t GetFlags() const override { return material_flags_; }
  virtual void SetFlags(const uint32_t flags) override {
    material_flags_ = flags;
  }
  virtual bool GetFlag(MaterialFlag flag) const override {
    return static_cast<uint32_t>(flag) & material_flags_;
  }
  virtual void SetFlag(MaterialFlag flag, bool value = true) override {
    if (value) {
      material_flags_ |= (uint32_t)flag;
    } else {
      material_flags_ &= ~(uint32_t)flag;
    }
  }

  virtual Ref<Shader> GetShader() override { return shader_; }

  VkDescriptorSet GetDescriptorSet(uint32_t index);
  Buffer GetUniformStorageBuffer() const;

 private:
  const ShaderUniform* FindUniformDeclaration(const std::string& name);

  template <typename T>
  void SetBuffer(const std::string& name, const T& value) {
    const auto decl = FindUniformDeclaration(name);
    BE_CORE_ASSERT(decl, "Could not find uniform!");
    if (!decl) return;

    auto& buffer = uniform_storage_buffer_;
    buffer.Write((uint8_t*)&value, decl->GetSize(),
                 decl->GetOffset());
  }

  template <typename T>
  T& GetBuffer(const std::string& name) {
    const auto decl = FindUniformDeclaration(name);
    BE_CORE_ASSERT(decl, "Could not find uniform!");
    auto& buffer = uniform_storage_buffer_;
    return buffer.Read<T>(decl->GetOffset());
  }

  void Init();
  void AllocateStorage();

 private:
  Ref<VulkanShader> shader_;
  std::string name_;

  DescriptorSetManager descriptor_set_manager_;
  std::vector<VkDescriptorSet> material_descriptor_sets_;

  Buffer uniform_storage_buffer_;

  uint32_t material_flags_ = 0;
};
}  // namespace base_engine

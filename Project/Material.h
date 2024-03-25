// @kMaterial.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <glm/glm.hpp>

#include "Ref.h"
#include "RendererTexture.h"
#include "Shader.h"
#include "Texture.h"
namespace base_engine {
enum class MaterialFlag {
  None = (1u << 0),
  DepthTest = (1u << 1),
  Blend = (1u << 2),
  TwoSided = (1u << 3),
  DisableShadowCasting = (1u << 4)
};

class Material : public RefCounted {
 public:
  virtual void Set(const std::string& name, float value) = 0;
  virtual void Set(const std::string& name, int value) = 0;
  virtual void Set(const std::string& name, uint32_t value) = 0;
  virtual void Set(const std::string& name, bool value) = 0;
  virtual void Set(const std::string& name, const glm::vec2& value) = 0;
  virtual void Set(const std::string& name, const glm::vec3& value) = 0;
  virtual void Set(const std::string& name, const glm::vec4& value) = 0;
  virtual void Set(const std::string& name, const glm::ivec2& value) = 0;
  virtual void Set(const std::string& name, const glm::ivec3& value) = 0;
  virtual void Set(const std::string& name, const glm::ivec4& value) = 0;

  virtual void Set(const std::string& name, const glm::mat3& value) = 0;
  virtual void Set(const std::string& name, const glm::mat4& value) = 0;

  virtual void Set(const std::string& name,
                   const Ref<RendererTexture2D>& texture) = 0;
  virtual void Set(const std::string& name,
                   const Ref<RendererTexture2D>& texture,
                   uint32_t arrayIndex) = 0;

  		virtual float& GetFloat(const std::string& name) = 0;
  virtual int32_t& GetInt(const std::string& name) = 0;
  virtual uint32_t& GetUInt(const std::string& name) = 0;
  virtual bool& GetBool(const std::string& name) = 0;
  virtual glm::vec2& GetVector2(const std::string& name) = 0;
  virtual glm::vec3& GetVector3(const std::string& name) = 0;
  virtual glm::vec4& GetVector4(const std::string& name) = 0;
  virtual glm::mat3& GetMatrix3(const std::string& name) = 0;
  virtual glm::mat4& GetMatrix4(const std::string& name) = 0;

  virtual Ref<RendererTexture2D> GetTexture2D(const std::string& name) = 0;
  virtual Ref<RendererTexture2D> TryGetTexture2D(const std::string& name) = 0;

  virtual uint32_t GetFlags() const = 0;
  virtual void SetFlags(uint32_t flags) = 0;

  virtual bool GetFlag(MaterialFlag flag) const = 0;
  virtual void SetFlag(MaterialFlag flag, bool emplace = true) = 0;

	virtual Ref<Shader> GetShader() = 0;

  static Ref<Material> Create(const Ref<Shader>& shader,
                              const std::string& name = "");
  static Ref<Material> Copy(const Ref<Material>& other,
                            const std::string& name = "");

 private:
  Ref<Shader> shader_;
  Ref<Texture> texture_;
};
}  // namespace base_engine

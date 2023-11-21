// @Shader.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include <functional>
#include <string>

#include "Ref.h"

namespace base_engine {

enum class ShaderUniformType {
  None = 0,
  Bool,
  Int,
  UInt,
  Float,
  Vec2,
  Vec3,
  Vec4,
  Mat3,
  Mat4,
  IVec2,
  IVec3,
  IVec4
};

class ShaderUniform {
 public:
  ShaderUniform() = default;
  ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size,
                uint32_t offset);

  [[nodiscard]] const std::string& GetName() const { return name_; }
  [[nodiscard]] ShaderUniformType GetType() const { return type_; }
  [[nodiscard]] uint32_t GetSize() const { return size_; }
  [[nodiscard]] uint32_t GetOffset() const { return offset_; }

 private:
  std::string name_;
  ShaderUniformType type_ = ShaderUniformType::None;
  uint32_t size_ = 0;
  uint32_t offset_ = 0;
};

struct ShaderBuffer {
  std::string Name;
  uint32_t Size = 0;
  std::unordered_map<std::string, ShaderUniform> Uniforms;
};
class Shader : public RefCounted {
 public:
  using ShaderReloadedCallback = std::function<void()>;

  virtual void Reload(bool forceCompile = false) = 0;
  virtual void RT_Reload(bool forceCompile) = 0;

  static Ref<Shader> Create(const std::string& filepath,
                            bool forceCompile = false,
                            bool disableOptimization = false);
};
}  // namespace base_engine
// @Material.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include "Ref.h"
#include "RendererTexture.h"
#include "Shader.h"
#include "Texture.h"

namespace base_engine {

class Material : public RefCounted {
 public:
  virtual void Set(const std::string& name,
                   const Ref<RendererTexture2D>& texture,
                   uint32_t arrayIndex) = 0;

  static Ref<Material> Create(const Ref<Shader>& shader,
                              const std::string& name);

 private:
  Ref<Shader> shader_;
  Ref<Texture> texture_;
};
}  // namespace base_engine

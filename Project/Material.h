// @Material.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include "Ref.h"
#include "Shader.h"
#include "Texture.h"

namespace base_engine {

class Material : public RefCounted {
 public:


  static Ref<Material> Create(const Ref<Shader>& shader,
                              const std::string& name);

private:
  Ref<Shader> shader_;
  Ref<Texture> texture_;
};
}  // namespace base_engine

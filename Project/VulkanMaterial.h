// @VulkanMaterial.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include "Buffer.h"
#include "Material.h"
#include "VulkanShader.h"

namespace base_engine {
class VulkanMaterial : public Material {
 public:
  VulkanMaterial() = default;
  explicit VulkanMaterial(const Ref<Shader>& shader,
                          const std::string& name = "");

 private:
  void Init();
  void AllocateStorage();

  Ref<VulkanShader> m_Shader;
  std::string m_Name;

  Buffer m_UniformStorageBuffer;
};
}  // namespace base_engine

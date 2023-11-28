// @VulkanMaterial.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include "Buffer.h"
#include "DescriptorSetManager.h"
#include "Material.h"
#include "VulkanShader.h"

namespace base_engine {
class VulkanMaterial : public Material {
 public:
  VulkanMaterial() = default;
  explicit VulkanMaterial(const Ref<Shader>& shader,
                          const std::string& name = "");
  void Set(const std::string& name, const Ref<RendererTexture2D>& texture,
           uint32_t arrayIndex) override;
  VkDescriptorSet GetDescriptorSet(uint32_t index);
  Buffer GetUniformStorageBuffer() const;

private:
  void Init();
  void AllocateStorage();

  Ref<VulkanShader> shader_;
  std::string name_;

  DescriptorSetManager descriptor_set_manager_;
  std::vector<VkDescriptorSet> material_descriptor_sets_;

  Buffer uniform_storage_buffer_;
};
}  // namespace base_engine

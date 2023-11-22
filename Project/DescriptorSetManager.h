// @DescriptorSetManager.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <cstdint>
#include <string>

#include "RendererTexture.h"
#include "UniformBuffer.h"
#include "UniformBufferSet.h"
#include "VulkanShader.h"

namespace base_engine {

enum class RenderPassResourceType : uint16_t {
  None = 0,
  UniformBuffer,
  UniformBufferSet,
  StorageBuffer,
  StorageBufferSet,
  Texture2D,
  TextureCube,
  Image2D
};

enum class RenderPassInputType : uint16_t {
  None = 0,
  UniformBuffer,
  StorageBuffer,
  ImageSampler1D,
  ImageSampler2D,
  ImageSampler3D,
  StorageImage1D,
  StorageImage2D,
  StorageImage3D
};

struct RenderPassInput {
  RenderPassResourceType Type = RenderPassResourceType::None;
  std::vector<Ref<RefCounted>> Input;

  RenderPassInput() = default;
  void Set(Ref<UniformBuffer> uniformBuffer, uint32_t index = 0) {
    Type = RenderPassResourceType::UniformBuffer;
    Input[index] = uniformBuffer;
  }

  void Set(Ref<UniformBufferSet> uniformBufferSet, uint32_t index = 0) {
    Type = RenderPassResourceType::UniformBufferSet;
    Input[index] = uniformBufferSet;
  }

  void Set(Ref<RendererTexture2D> texture, uint32_t index = 0) {
    Type = RenderPassResourceType::Texture2D;
    Input[index] = texture;
  }
};

struct DescriptorSetManagerSpecification {
  Ref<VulkanShader> Shader;
  std::string DebugName;

  uint32_t StartSet = 0, EndSet = 2;
  bool DefaultResources = false;
};

struct RenderPassInputDeclaration {
  RenderPassInputType Type = RenderPassInputType::None;
  uint32_t Set = 0;
  uint32_t Binding = 0;
  uint32_t Count = 0;
  std::string Name;
};
class DescriptorSetManager {
 public:
  explicit DescriptorSetManager(
      DescriptorSetManagerSpecification specification);

 private:
  void Init();

  std::map<std::string, RenderPassInputDeclaration> InputDeclarations;

  struct WriteDescriptor {
    VkWriteDescriptorSet WriteDescriptorSet{};
    std::vector<void*> ResourceHandles;
  };
  std::vector<std::map<uint32_t, std::map<uint32_t, WriteDescriptor>>>
      WriteDescriptorMap;

  DescriptorSetManagerSpecification m_Specification;
  VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
};
}  // namespace base_engine

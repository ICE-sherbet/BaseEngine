// @DescriptorSetManager.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <cstdint>
#include <set>
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
  Image2D,
  AccelerationStructure
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
  StorageImage3D,
  AccelerationStructure
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

  void Set(Ref<Image2D> image, uint32_t index = 0) {
    Type = RenderPassResourceType::Image2D;
    Input[index] = image;
  }

  void Set(Ref<ImageView> image, uint32_t index = 0) {
    Type = RenderPassResourceType::Image2D;
    Input[index] = image;
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

struct DescriptorSetManager {
  DescriptorSetManager() = default;

  explicit DescriptorSetManager(
      DescriptorSetManagerSpecification specification);

  void SetInput(std::string_view name,
                const Ref<UniformBufferSet>& uniform_buffer_set);
  void SetInput(std::string_view name,
                const Ref<UniformBuffer>& uniform_buffer);
  void SetInput(std::string_view name, const Ref<RendererTexture2D>& texture,
                uint32_t index = 0);
  void SetInput(std::string_view name, const Ref<Image2D>& image);
  void SetInput(std::string_view name, const Ref<ImageView>& image);

  const RenderPassInputDeclaration* GetInputDeclaration(
      std::string_view name) const;
  bool IsInvalidated(uint32_t set, uint32_t binding) const;
  uint32_t GetFirstSetIndex() const;
  const std::vector<VkDescriptorSet>& GetDescriptorSets(
      uint32_t frame_index) const;
  bool IsInputValid(std::string_view name) const;
  void Bake();
  bool HasDescriptorSets() const;
  VkDescriptorPool GetDescriptorPool() const { return descriptor_pool; }

  static bool IsCompatibleInput(const RenderPassResourceType input,
                                const VkDescriptorType vk_descriptor) {
    switch (vk_descriptor) {
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
        return input == RenderPassResourceType::Texture2D ||
               input == RenderPassResourceType::TextureCube ||
               input == RenderPassResourceType::Image2D;
      }
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
        return input == RenderPassResourceType::Image2D;
      }
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
        return input == RenderPassResourceType::UniformBuffer ||
               input == RenderPassResourceType::UniformBufferSet;
      }
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
        return input == RenderPassResourceType::StorageBuffer ||
               input == RenderPassResourceType::StorageBufferSet;
      }
    }
    return false;
  }

  bool Validate();

  std::set<uint32_t> HasBufferSets() const;
  void InvalidateAndUpdate();

  template <typename T>
  Ref<T> GetInput(const std::string_view name) {
    if (const RenderPassInputDeclaration* decl = GetInputDeclaration(name)) {
      if (const auto set_it = InputResources.find(decl->Set);
          set_it != InputResources.end()) {
        if (const auto resource_it = set_it->second.find(decl->Binding);
            resource_it != set_it->second.end())
          return resource_it->second.Input[0].As<T>();
      }
    }
    return nullptr;
  }

 private:
  template <typename T, typename... Args>
  void SetInputImpl(std::string_view name, const Ref<T>& item, Args... args);
  void Init();

 public:
  std::vector<std::vector<VkDescriptorSet>> descriptor_sets;

  std::map<uint32_t, std::map<uint32_t, RenderPassInput>> InputResources;
  std::map<uint32_t, std::map<uint32_t, RenderPassInput>>
      InvalidatedInputResources;

  std::map<std::string, RenderPassInputDeclaration> InputDeclarations;

  struct WriteDescriptor {
    VkWriteDescriptorSet WriteDescriptorSet{};
    std::vector<void*> ResourceHandles;
  };
  std::vector<std::map<uint32_t, std::map<uint32_t, WriteDescriptor>>>
      WriteDescriptorMap;

  DescriptorSetManagerSpecification specification;
  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
};

}  // namespace base_engine

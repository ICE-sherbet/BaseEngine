#include "DescriptorSetManager.h"

#include <ranges>

#include "RendererApi.h"
#include "VulkanAPI.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "VulkanUniformBuffer.h"
#include "VulkanUniformBufferSet.h"

namespace base_engine {
namespace {
RenderPassInputType RenderPassInputTypeFromVulkanDescriptorType(
    VkDescriptorType descriptorType) {
  switch (descriptorType) {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      return RenderPassInputType::ImageSampler2D;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return RenderPassInputType::StorageImage2D;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      return RenderPassInputType::UniformBuffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return RenderPassInputType::StorageBuffer;
  }

  return RenderPassInputType::None;
}

RenderPassResourceType GetDefaultResourceType(VkDescriptorType descriptorType) {
  switch (descriptorType) {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      return RenderPassResourceType::Texture2D;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return RenderPassResourceType::Image2D;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      return RenderPassResourceType::UniformBuffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return RenderPassResourceType::StorageBuffer;
  }

  return RenderPassResourceType::None;
}
}  // namespace

DescriptorSetManager::DescriptorSetManager(
    DescriptorSetManagerSpecification specification)
    : specification(std::move(specification)) {
  Init();
}

template <class T, class... Args>
void DescriptorSetManager::SetInputImpl(std::string_view name,
                                        const Ref<T>& item, Args... args) {
  const RenderPassInputDeclaration* decl = GetInputDeclaration(name);
  if (decl)
    InputResources.at(decl->Set).at(decl->Binding).Set(item, args...);
  else
    BE_CORE_WARN_TAG("Renderer", "[RenderPass ({})] Input {} not found",
                     specification.DebugName, name);
}
template void DescriptorSetManager::SetInputImpl<UniformBufferSet>(
    std::string_view name, const Ref<UniformBufferSet>& item);

template void DescriptorSetManager::SetInputImpl<UniformBuffer>(
    std::string_view name, const Ref<UniformBuffer>& item);
template void DescriptorSetManager::SetInputImpl<RendererTexture2D, uint32_t>(
    std::string_view name, const Ref<RendererTexture2D>& item, uint32_t index);
template void DescriptorSetManager::SetInputImpl<Image2D>(
    std::string_view name, const Ref<Image2D>& item);
template void DescriptorSetManager::SetInputImpl<ImageView>(
    std::string_view name, const Ref<ImageView>& item);

void DescriptorSetManager::SetInput(
    std::string_view name, const Ref<UniformBufferSet>& uniform_buffer_set) {
  SetInputImpl(name, uniform_buffer_set);
}

void DescriptorSetManager::SetInput(const std::string_view name,
                                    const Ref<UniformBuffer>& uniform_buffer) {
  SetInputImpl(name, uniform_buffer);
}

void DescriptorSetManager::SetInput(const std::string_view name,
                                    const Ref<RendererTexture2D>& texture,
                                    const uint32_t index) {
  SetInputImpl(name, texture, index);
}

void DescriptorSetManager::SetInput(const std::string_view name,
                                    const Ref<Image2D>& image) {
  SetInputImpl(name, image);
}

void DescriptorSetManager::SetInput(const std::string_view name,
                                    const Ref<ImageView>& image) {
  SetInputImpl(name, image);
}

const RenderPassInputDeclaration* DescriptorSetManager::GetInputDeclaration(
    const std::string_view name) const {
  const std::string name_str(name);
  if (!InputDeclarations.contains(name_str)) return nullptr;

  const RenderPassInputDeclaration& decl = InputDeclarations.at(name_str);
  return &decl;
}

bool DescriptorSetManager::IsInvalidated(const uint32_t set,
                                         const uint32_t binding) const {
  if (InvalidatedInputResources.contains(set)) {
    const auto& resources = InvalidatedInputResources.at(set);
    return resources.contains(binding);
  }

  return false;
}

uint32_t DescriptorSetManager::GetFirstSetIndex() const {
  if (InputResources.empty()) return (std::numeric_limits<uint32_t>::max)();
  return InputResources.begin()->first;
}

const std::vector<VkDescriptorSet>& DescriptorSetManager::GetDescriptorSets(
    const uint32_t frame_index) const {
  if (frame_index > 0 && descriptor_sets.size() == 1)
    return descriptor_sets[0];

  return descriptor_sets[frame_index];
}

bool DescriptorSetManager::IsInputValid(std::string_view name) const {
  const std::string name_str(name);
  return InputDeclarations.contains(name_str);
}

void DescriptorSetManager::Bake() {
  if (!Validate()) {
    BE_CORE_ERROR_TAG("Renderer", "[RenderPass] Bake - Validate failed! {}",
                      specification.DebugName);
    return;
  }

  // If valid, we can create descriptor sets

  // Create Descriptor Pool
  VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets =
      10 * 3;  // frames in flight should partially determine this
  poolInfo.poolSizeCount = 10;
  poolInfo.pPoolSizes = poolSizes;

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

  vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptor_pool);

  auto bufferSets = HasBufferSets();
  bool perFrameInFlight = !bufferSets.empty();
  perFrameInFlight = true;  // always
  uint32_t descriptorSetCount = 3;
  if (!perFrameInFlight) descriptorSetCount = 1;

  if (descriptor_sets.empty()) {
    for (uint32_t i = 0; i < descriptorSetCount; i++)
      descriptor_sets.emplace_back();
  }

  for (auto& descriptorSet : descriptor_sets) descriptorSet.clear();

  for (const auto& [set, setData] : InputResources) {
    for (uint32_t frame_index = 0; frame_index < descriptorSetCount;
         frame_index++) {
      VkDescriptorSetLayout dsl =
          specification.Shader->GetDescriptorSetLayout(set);
      VkDescriptorSetAllocateInfo descriptorSetAllocInfo =
          vulkan::DescriptorSetAllocInfo(&dsl);
      descriptorSetAllocInfo.descriptorPool = descriptor_pool;
      VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
      vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSet);

      descriptor_sets[frame_index].emplace_back(descriptorSet);

      auto& writeDescriptorMap = WriteDescriptorMap[frame_index].at(set);
      std::vector<std::vector<VkDescriptorImageInfo>> imageInfoStorage;
      uint32_t imageInfoStorageIndex = 0;

      for (const auto& [binding, input] : setData) {
        auto& storedWriteDescriptor = writeDescriptorMap.at(binding);

        VkWriteDescriptorSet& writeDescriptor =
            storedWriteDescriptor.WriteDescriptorSet;
        writeDescriptor.dstSet = descriptorSet;

        switch (input.Type) {
          case RenderPassResourceType::UniformBuffer: {
            Ref<VulkanUniformBuffer> buffer =
                input.Input[0].As<VulkanUniformBuffer>();
            writeDescriptor.pBufferInfo = &buffer->GetDescriptorBufferInfo();
            storedWriteDescriptor.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pBufferInfo->buffer);
            if (writeDescriptor.pBufferInfo->buffer == VK_NULL_HANDLE)
              InvalidatedInputResources[set][binding] = input;

            break;
          }
          case RenderPassResourceType::UniformBufferSet: {
            Ref<UniformBufferSet> buffer =
                input.Input[0].As<UniformBufferSet>();
            writeDescriptor.pBufferInfo = &buffer->Get(frame_index)
                                               .As<VulkanUniformBuffer>()
                                               ->GetDescriptorBufferInfo();
            storedWriteDescriptor.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pBufferInfo->buffer);

            if (writeDescriptor.pBufferInfo->buffer == VK_NULL_HANDLE)
              InvalidatedInputResources[set][binding] = input;

            break;
          }
          case RenderPassResourceType::Texture2D: {
            if (input.Input.size() > 1) {
              imageInfoStorage.emplace_back(input.Input.size());
              for (size_t i = 0; i < input.Input.size(); i++) {
                Ref<VulkanTexture2D> texture =
                    input.Input[i].As<VulkanTexture2D>();
                imageInfoStorage[imageInfoStorageIndex][i] =
                    texture->GetDescriptorInfoVulkan();
              }
              writeDescriptor.pImageInfo =
                  imageInfoStorage[imageInfoStorageIndex].data();
              imageInfoStorageIndex++;
            } else {
              Ref<VulkanTexture2D> texture =
                  input.Input[0].As<VulkanTexture2D>();
              writeDescriptor.pImageInfo = &texture->GetDescriptorInfoVulkan();
            }
            storedWriteDescriptor.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pImageInfo->imageView);

            if (writeDescriptor.pImageInfo->imageView == VK_NULL_HANDLE)
              InvalidatedInputResources[set][binding] = input;

            break;
          }
          case RenderPassResourceType::Image2D: {
            Ref<RendererResource> image = input.Input[0].As<RendererResource>();
            if (image == nullptr) {
              InvalidatedInputResources[set][binding] = input;
              break;
            }

            writeDescriptor.pImageInfo =
                static_cast<VkDescriptorImageInfo*>(image->GetDescriptorInfo());
            storedWriteDescriptor.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pImageInfo->imageView);

            if (writeDescriptor.pImageInfo->imageView == VK_NULL_HANDLE)
              InvalidatedInputResources[set][binding] = input;

            break;
          }
        }
      }

      std::vector<VkWriteDescriptorSet> writeDescriptors;
      for (auto&& [binding, writeDescriptor] : writeDescriptorMap) {
        if (!IsInvalidated(set, binding))
          writeDescriptors.emplace_back(writeDescriptor.WriteDescriptorSet);
      }

      if (!writeDescriptors.empty()) {
        BE_CORE_INFO_TAG("Renderer",
                         "Render pass update {} descriptors in set {}",
                         writeDescriptors.size(), set);
        vkUpdateDescriptorSets(device, (uint32_t)writeDescriptors.size(),
                               writeDescriptors.data(), 0, nullptr);
      }
    }
  }
}

bool DescriptorSetManager::HasDescriptorSets() const
{
  return !descriptor_sets.empty() && !descriptor_sets[0].empty();
}

bool DescriptorSetManager::Validate() {
  const auto& shaderDescriptorSets =
      specification.Shader->GetShaderDescriptorSets();

  for (uint32_t set = specification.StartSet; set <= specification.EndSet;
       set++) {
    if (set >= shaderDescriptorSets.size()) break;

    // No descriptors in this set
    if (!shaderDescriptorSets[set]) continue;

    if (InputResources.find(set) == InputResources.end()) {
      BE_CORE_ERROR_TAG("Renderer",
                        "[RenderPass ({})] No input resources for Set {}",
                        specification.DebugName, set);
      return false;
    }

    const auto& setInputResources = InputResources.at(set);

    const auto& shaderDescriptor = shaderDescriptorSets[set];
    for (auto&& [name, wd] : shaderDescriptor.WriteDescriptorSets) {
      uint32_t binding = wd.dstBinding;
      if (setInputResources.find(binding) == setInputResources.end()) {
        BE_CORE_ERROR_TAG("Renderer",
                          "[RenderPass ({})] No input resource for {}.{}",
                          specification.DebugName, set, binding);
        BE_CORE_ERROR_TAG("Renderer",
                          "[RenderPass ({})] Required resource is {} ({})",
                          specification.DebugName, name, wd.descriptorType);
        return false;
      }

      const auto& resource = setInputResources.at(binding);
      if (!IsCompatibleInput(resource.Type, wd.descriptorType)) {
        BE_CORE_ERROR_TAG("Renderer",
                          "[RenderPass ({})] Required resource is wrong type! "
                          "{} but needs {}",
                          specification.DebugName, resource.Type,
                          wd.descriptorType);
        return false;
      }

      if (resource.Type != RenderPassResourceType::Image2D &&
          resource.Input[0] == nullptr) {
        BE_CORE_ERROR_TAG("Renderer",
                          "[RenderPass ({})] Resource is null! {} ({}.{})",
                          specification.DebugName, name, set, binding);
        return false;
      }
    }
  }

  return true;
}

std::set<uint32_t> DescriptorSetManager::HasBufferSets() const {
  std::set<uint32_t> sets;

  for (const auto& [set, resources] : InputResources) {
    for (const auto& input : resources | std::views::values) {
      if (input.Type == RenderPassResourceType::UniformBufferSet ||
          input.Type == RenderPassResourceType::StorageBufferSet) {
        sets.insert(set);
        break;
      }
    }
  }
  return sets;
}

void DescriptorSetManager::InvalidateAndUpdate() {
  uint32_t currentFrameIndex = Renderer::RT_GetCurrentFrameIndex();

  // Check for invalidated resources
  for (const auto& [set, inputs] : InputResources) {
    for (const auto& [binding, input] : inputs) {
      switch (input.Type) {
        case RenderPassResourceType::UniformBuffer: {
          // for (uint32_t frameIndex = 0; frameIndex <
          // (uint32_t)WriteDescriptorMap.size(); frameIndex++)
          {
            const VkDescriptorBufferInfo& bufferInfo =
                input.Input[0]
                    .As<VulkanUniformBuffer>()
                    ->GetDescriptorBufferInfo();
            if (reinterpret_cast<void*>(bufferInfo.buffer) !=
                WriteDescriptorMap[currentFrameIndex]
                    .at(set)
                    .at(binding)
                    .ResourceHandles[0]) {
              InvalidatedInputResources[set][binding] = input;
              break;
            }
          }
          break;
        }
        case RenderPassResourceType::UniformBufferSet: {
          {
            const VkDescriptorBufferInfo& bufferInfo =
                input.Input[0]
                    .As<VulkanUniformBufferSet>()
                    ->Get(currentFrameIndex)
                    .As<VulkanUniformBuffer>()
                    ->GetDescriptorBufferInfo();
            auto rh = WriteDescriptorMap[currentFrameIndex]
                          .at(set)
                          .at(binding)
                          .ResourceHandles[0];
            if (reinterpret_cast<void*>(bufferInfo.buffer) ==
                WriteDescriptorMap[currentFrameIndex]
                    .at(set)
                    .at(binding)
                    .ResourceHandles[0]) {
              InvalidatedInputResources[set][binding] = input;
              break;
            }
          }
          break;
        }
        case RenderPassResourceType::Texture2D: {
          for (size_t i = 0; i < input.Input.size(); i++) {
            const VkDescriptorImageInfo& imageInfo =
                input.Input[i].As<VulkanTexture2D>()->GetDescriptorInfoVulkan();
            if (reinterpret_cast<void*>(imageInfo.imageView) !=
                WriteDescriptorMap[currentFrameIndex]
                    .at(set)
                    .at(binding)
                    .ResourceHandles[i]) {
              InvalidatedInputResources[set][binding] = input;
              break;
            }
          }
          break;
        }
        case RenderPassResourceType::Image2D: {
          {
            const VkDescriptorImageInfo& image_info =
                *static_cast<VkDescriptorImageInfo*>(
                    input.Input[0].As<RendererResource>()->GetDescriptorInfo());
            if (reinterpret_cast<void*>(image_info.imageView) !=
                WriteDescriptorMap[currentFrameIndex]
                    .at(set)
                    .at(binding)
                    .ResourceHandles[0]) {
              InvalidatedInputResources[set][binding] = input;
              break;
            }
          }
          break;
        }
      }
    }
  }

  if (InvalidatedInputResources.empty()) return;

  auto bufferSets = HasBufferSets();
  const bool perFrameInFlight = true;
  uint32_t descriptor_set_count = 3;
  if (!perFrameInFlight) descriptor_set_count = 1;

  for (const auto& [set, setData] : InvalidatedInputResources) {
    const uint32_t frame_index = perFrameInFlight ? currentFrameIndex : 0;
    {
      std::vector<VkWriteDescriptorSet> writeDescriptorsToUpdate;
      writeDescriptorsToUpdate.reserve(setData.size());
      std::vector<std::vector<VkDescriptorImageInfo>> imageInfoStorage;
      uint32_t imageInfoStorageIndex = 0;
      for (const auto& [binding, input] : setData) {
        auto& wd = WriteDescriptorMap[frame_index].at(set).at(binding);
        VkWriteDescriptorSet& writeDescriptor = wd.WriteDescriptorSet;
        switch (input.Type) {
          case RenderPassResourceType::UniformBuffer: {
            Ref<VulkanUniformBuffer> buffer =
                input.Input[0].As<VulkanUniformBuffer>();
            writeDescriptor.pBufferInfo = &buffer->GetDescriptorBufferInfo();
            wd.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pBufferInfo->buffer);
            break;
          }
          case RenderPassResourceType::UniformBufferSet: {
            Ref<UniformBufferSet> buffer =
                input.Input[0].As<UniformBufferSet>();
            writeDescriptor.pBufferInfo = &buffer->Get(frame_index)
                                               .As<VulkanUniformBuffer>()
                                               ->GetDescriptorBufferInfo();
            wd.ResourceHandles[0] =
                reinterpret_cast<void*>(writeDescriptor.pBufferInfo->buffer);
            break;
          }
          case RenderPassResourceType::Texture2D: {
            if (input.Input.size() > 1) {
              imageInfoStorage.emplace_back(input.Input.size());
              for (size_t i = 0; i < input.Input.size(); i++) {
                Ref<VulkanTexture2D> texture =
                    input.Input[i].As<VulkanTexture2D>();
                imageInfoStorage[imageInfoStorageIndex][i] =
                    texture->GetDescriptorInfoVulkan();
                wd.ResourceHandles[i] = reinterpret_cast<void*>(
                    imageInfoStorage[imageInfoStorageIndex][i].imageView);
              }
              writeDescriptor.pImageInfo =
                  imageInfoStorage[imageInfoStorageIndex].data();
              imageInfoStorageIndex++;
            } else {
              Ref<VulkanTexture2D> texture =
                  input.Input[0].As<VulkanTexture2D>();
              writeDescriptor.pImageInfo = &texture->GetDescriptorInfoVulkan();
              wd.ResourceHandles[0] = reinterpret_cast<void*>(writeDescriptor.pImageInfo->imageView);
            }

            break;
          }
          case RenderPassResourceType::Image2D: {
            Ref<RendererResource> image = input.Input[0].As<RendererResource>();
            writeDescriptor.pImageInfo =
                (VkDescriptorImageInfo*)image->GetDescriptorInfo();
            wd.ResourceHandles[0] = reinterpret_cast<void*>(writeDescriptor.pImageInfo->imageView);
            break;
          }
        }
        writeDescriptorsToUpdate.emplace_back(writeDescriptor);
      }
      VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
      vkUpdateDescriptorSets(device, (uint32_t)writeDescriptorsToUpdate.size(),
                             writeDescriptorsToUpdate.data(), 0, nullptr);
    }
  }

  InvalidatedInputResources.clear();
}

void DescriptorSetManager::Init() {
  const auto& shaderDescriptorSets =
      specification.Shader->GetShaderDescriptorSets();
  uint32_t framesInFlight = 3;
  WriteDescriptorMap.resize(framesInFlight);

  for (uint32_t set = specification.StartSet; set <= specification.EndSet;
       set++) {
    if (set >= shaderDescriptorSets.size()) break;

    for (const auto& shader_descriptor = shaderDescriptorSets[set];
         auto&& [bname, wd] : shader_descriptor.WriteDescriptorSets) {
      const char* broken = strrchr(bname.c_str(), '.');
      std::string name = broken ? broken + 1 : bname;

      uint32_t binding = wd.dstBinding;
      RenderPassInputDeclaration& inputDecl = InputDeclarations[name];
      inputDecl.Type =
          RenderPassInputTypeFromVulkanDescriptorType(wd.descriptorType);
      inputDecl.Set = set;
      inputDecl.Binding = binding;
      inputDecl.Name = name;
      inputDecl.Count = wd.descriptorCount;

      if (specification.DefaultResources || true) {
        RenderPassInput& input = InputResources[set][binding];
        input.Input.resize(wd.descriptorCount);
        input.Type = GetDefaultResourceType(wd.descriptorType);

        // Set default textures
        if (inputDecl.Type == RenderPassInputType::ImageSampler2D) {
          for (size_t i = 0; i < input.Input.size(); i++) {
            input.Input[i] = Renderer::GetWhiteTexture();
          }
        }
      }

      for (uint32_t frameIndex = 0; frameIndex < framesInFlight; frameIndex++)
        WriteDescriptorMap[frameIndex][set][binding] = {
            wd, std::vector<void*>(wd.descriptorCount)};
    }
  }
}
}  // namespace base_engine
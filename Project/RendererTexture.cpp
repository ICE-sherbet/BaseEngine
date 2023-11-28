#include "RendererTexture.h"

#include "RendererApi.h"
#include "VulkanTexture.h"

namespace base_engine {
Ref<RendererTexture2D> RendererTexture2D::Create(
    const TextureSpecification& specification) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanTexture2D>::Create(specification);
  }
  BE_CORE_ASSERT(false, "Unknown RendererApi");
  return nullptr;
}

Ref<RendererTexture2D> RendererTexture2D::Create(
    const TextureSpecification& specification,
    const std::filesystem::path& filepath) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanTexture2D>::Create(specification, filepath);
  }
  BE_CORE_ASSERT(false, "Unknown RendererApi");
  return nullptr;
}

Ref<RendererTexture2D> RendererTexture2D::Create(
    const TextureSpecification& specification, Buffer imageData) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanTexture2D>::Create(specification, imageData);
  }
  BE_CORE_ASSERT(false, "Unknown RendererApi");
  return nullptr;
}
}  // namespace base_engine

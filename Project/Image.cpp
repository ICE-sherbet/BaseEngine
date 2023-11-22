#include "Image.h"

#include "RendererApi.h"
#include "VulkanImage.h"

namespace base_engine
{

Ref<Image2D> Image2D::Create(const ImageSpecification& specification,
                             Buffer buffer) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanImage2D>::Create(specification);
  }
  return nullptr;
}

Ref<Image2D> Image2D::Create(const ImageSpecification& specification,
                             const void* data) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanImage2D>::Create(specification);
  }
  return nullptr;
}

Ref<ImageView> ImageView::Create(const ImageViewSpecification& specification) {
  switch (RendererApi::Current()) {
    case RendererApiType::kNone:
      return nullptr;
    case RendererApiType::kVulkan:
      return Ref<VulkanImageView>::Create(specification);
  }
  return nullptr;
}
}

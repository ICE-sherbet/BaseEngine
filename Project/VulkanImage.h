// @VulkanImage.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <map>

#include "Buffer.h"
#include "Image.h"
#include "Ref.h"
#include "VulkanContext.h"

namespace base_engine {
struct VulkanImageInfo {
  VkImage Image = VK_NULL_HANDLE;
  VkImageView ImageView = VK_NULL_HANDLE;
  VkSampler Sampler = VK_NULL_HANDLE;
  VmaAllocation MemoryAlloc = nullptr;
};

class VulkanImage2D : public Image2D {
 public:
  VulkanImage2D(const ImageSpecification& specification);
  virtual ~VulkanImage2D() override;

  virtual void Resize(const uint32_t width, const uint32_t height) override {
    specification_.Width = width;
    specification_.Height = height;
    Invalidate();
  }
  virtual void Invalidate() override;
  virtual void Release() override;

  virtual uint32_t GetWidth() const override { return specification_.Width; }
  virtual uint32_t GetHeight() const override { return specification_.Height; }

  virtual float GetAspectRatio() const override {
    return (float)specification_.Width / (float)specification_.Height;
  }

  virtual ImageSpecification& GetSpecification() override {
    return specification_;
  }
  virtual const ImageSpecification& GetSpecification() const override {
    return specification_;
  }

  void RT_Invalidate();

  virtual void CreatePerLayerImageViews() override;
  void RT_CreatePerLayerImageViews();
  void RT_CreatePerSpecificLayerImageViews(
      const std::vector<uint32_t>& layerIndices);

  virtual VkImageView GetLayerImageView(uint32_t layer) {
    BE_CORE_ASSERT(layer < per_layer_image_views_.size());
    return per_layer_image_views_[layer];
  }

  VkImageView GetMipImageView(uint32_t mip);
  VkImageView RT_GetMipImageView(uint32_t mip);

  VulkanImageInfo& GetImageInfo() { return info_; }
  const VulkanImageInfo& GetImageInfo() const { return info_; }

  virtual ResourceDescriptorInfo GetDescriptorInfo() const override {
    return (ResourceDescriptorInfo)&descriptor_image_info_;
  }
  const VkDescriptorImageInfo& GetDescriptorInfoVulkan() const {
    return *(VkDescriptorImageInfo*)GetDescriptorInfo();
  }

  virtual Buffer GetBuffer() const override { return image_data_; }
  virtual Buffer& GetBuffer() override { return image_data_; }

  virtual uint64_t GetHash() const override { return (uint64_t)info_.Image; }

  void UpdateDescriptor();

  static const std::map<VkImage, WeakRef<VulkanImage2D>>& GetImageRefs();

  void CopyToHostBuffer(Buffer& buffer);

 private:
  ImageSpecification specification_;

  Buffer image_data_;

  VulkanImageInfo info_;
  VkDeviceSize gpu_allocation_size_;

  std::vector<VkImageView> per_layer_image_views_;
  std::map<uint32_t, VkImageView> per_mip_image_views_;
  VkDescriptorImageInfo descriptor_image_info_ = {};
};

class VulkanImageView : public ImageView {
 public:
  VulkanImageView(const ImageViewSpecification& specification);
  virtual ~VulkanImageView();

  void Invalidate();
  void RT_Invalidate();

  VkImageView GetImageView() const { return image_view_; }

  virtual ResourceDescriptorInfo GetDescriptorInfo() const override {
    return (ResourceDescriptorInfo)&descriptor_image_info_;
  }
  const VkDescriptorImageInfo& GetDescriptorInfoVulkan() const {
    return *(VkDescriptorImageInfo*)GetDescriptorInfo();
  }

 private:
  ImageViewSpecification specification_;
  VkImageView image_view_ = VK_NULL_HANDLE;

  VkDescriptorImageInfo descriptor_image_info_ = {};
};
namespace Utils {

static VkFormat VulkanImageFormat(const ImageFormat format) {
  switch (format) {
    case ImageFormat::RED8UN:
      return VK_FORMAT_R8_UNORM;
    case ImageFormat::RED8UI:
      return VK_FORMAT_R8_UINT;
    case ImageFormat::RED16UI:
      return VK_FORMAT_R16_UINT;
    case ImageFormat::RED32UI:
      return VK_FORMAT_R32_UINT;
    case ImageFormat::RED32F:
      return VK_FORMAT_R32_SFLOAT;
    case ImageFormat::RG8:
      return VK_FORMAT_R8G8_UNORM;
    case ImageFormat::RG16F:
      return VK_FORMAT_R16G16_SFLOAT;
    case ImageFormat::RG32F:
      return VK_FORMAT_R32G32_SFLOAT;
    case ImageFormat::RGBA:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::RGBA16F:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ImageFormat::RGBA32F:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ImageFormat::B10R11G11UF:
      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case ImageFormat::DEPTH32FSTENCIL8UINT:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case ImageFormat::DEPTH32F:
      return VK_FORMAT_D32_SFLOAT;
    case ImageFormat::DEPTH24STENCIL8:
      return VulkanContext::GetCurrentDevice()
          ->GetPhysicalDevice()
          ->GetDepthFormat();
  }
  return VK_FORMAT_UNDEFINED;
}

}  // namespace Utils
}  // namespace base_engine

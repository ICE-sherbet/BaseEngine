#include "VulkanImage.h"

#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanAPI.h"
#include "VulkanContext.h"
#include "VulkanUtilities.h"

namespace base_engine
{

static std::map<VkImage, WeakRef<VulkanImage2D>> s_ImageReferences;

VulkanImage2D::VulkanImage2D(const ImageSpecification& specification)
    : specification_(specification) {
  BE_CORE_VERIFY(specification_.Width > 0 && specification_.Height > 0);
}

VulkanImage2D::~VulkanImage2D() { Release(); }

void VulkanImage2D::Invalidate() {
  Ref<VulkanImage2D> instance = this;
  Renderer::Submit([instance]() mutable { instance->RT_Invalidate(); });
}

void VulkanImage2D::Release() {
  if (info_.Image == VK_NULL_HANDLE) return;

  const VulkanImageInfo& info = info_;
  Renderer::SubmitResourceFree([info, mipViews = per_mip_image_views_,
                                layerViews = per_layer_image_views_]() mutable {
    const auto vulkanDevice =
        VulkanContext::GetCurrentDevice()->GetVulkanDevice();
    vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
    vulkan::DestroySampler(info.Sampler);

    for (auto& view : mipViews) {
      if (view.second) vkDestroyImageView(vulkanDevice, view.second, nullptr);
    }
    for (auto& view : layerViews) {
      if (view) vkDestroyImageView(vulkanDevice, view, nullptr);
    }
    VulkanAllocator allocator("VulkanImage2D");
    allocator.DestroyImage(info.Image, info.MemoryAlloc);
    s_ImageReferences.erase(info.Image);
  });
  info_.Image = VK_NULL_HANDLE;
  info_.ImageView = VK_NULL_HANDLE;
  if (specification_.CreateSampler) info_.Sampler = VK_NULL_HANDLE;
  per_layer_image_views_.clear();
  per_mip_image_views_.clear();
}

void VulkanImage2D::RT_Invalidate() {
  BE_CORE_VERIFY(specification_.Width > 0 && specification_.Height > 0);

  // Try release first if necessary
  Release();

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
  VulkanAllocator allocator("Image2D");

  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_SAMPLED_BIT;  // TODO: this (probably) shouldn't be implied
  if (specification_.Usage == ImageUsage::Attachment) {
    if (Utils::IsDepthFormat(specification_.Format))
      usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    else
      usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (specification_.Transfer ||
      specification_.Usage == ImageUsage::Texture) {
    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if (specification_.Usage == ImageUsage::Storage) {
    usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  VkImageAspectFlags aspectMask = Utils::IsDepthFormat(specification_.Format)
                                      ? VK_IMAGE_ASPECT_DEPTH_BIT
                                      : VK_IMAGE_ASPECT_COLOR_BIT;
  if (specification_.Format == ImageFormat::DEPTH24STENCIL8)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  VkFormat vulkanFormat = Utils::VulkanImageFormat(specification_.Format);

  VmaMemoryUsage memoryUsage = specification_.Usage == ImageUsage::HostRead
                                   ? VMA_MEMORY_USAGE_GPU_TO_CPU
                                   : VMA_MEMORY_USAGE_GPU_ONLY;

  VkImageCreateInfo imageCreateInfo = {};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = vulkanFormat;
  imageCreateInfo.extent.width = specification_.Width;
  imageCreateInfo.extent.height = specification_.Height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = specification_.Mips;
  imageCreateInfo.arrayLayers = specification_.Layers;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling = specification_.Usage == ImageUsage::HostRead
                               ? VK_IMAGE_TILING_LINEAR
                               : VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage = usage;
  info_.MemoryAlloc = allocator.AllocateImage(
      imageCreateInfo, memoryUsage, info_.Image, &gpu_allocation_size_);
  s_ImageReferences[info_.Image] = this;
  vulkan::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE,
                                   specification_.DebugName, info_.Image);

  // Create a default image view
  VkImageViewCreateInfo imageViewCreateInfo = {};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.viewType = specification_.Layers > 1
                                     ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                     : VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = vulkanFormat;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.subresourceRange = {};
  imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = specification_.Mips;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = specification_.Layers;
  imageViewCreateInfo.image = info_.Image;
  vkCreateImageView(device, &imageViewCreateInfo, nullptr,
                                    &info_.ImageView);
  vulkan::SetDebugUtilsObjectName(
      device, VK_OBJECT_TYPE_IMAGE_VIEW,
      fmt::format("{} default image view", specification_.DebugName),
      info_.ImageView);

  if (specification_.CreateSampler) {
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    if (Utils::IsIntegerBased(specification_.Format)) {
      samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
      samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
      samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    } else {
      samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
      samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
      samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }

    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
    samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 100.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    info_.Sampler = vulkan::CreateSampler(samplerCreateInfo);
    vulkan::SetDebugUtilsObjectName(
        device, VK_OBJECT_TYPE_SAMPLER,
        fmt::format("{} default sampler", specification_.DebugName),
        info_.Sampler);
  }

  if (specification_.Usage == ImageUsage::Storage) {
    // Transition image to GENERAL layout
    VkCommandBuffer commandBuffer =
        VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = specification_.Mips;
    subresourceRange.layerCount = specification_.Layers;

    vulkan::InsertImageMemoryBarrier(
        commandBuffer, info_.Image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, subresourceRange);

    VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
  } else if (specification_.Usage == ImageUsage::HostRead) {
    // Transition image to TRANSFER_DST layout
    VkCommandBuffer commandBuffer =
        VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = specification_.Mips;
    subresourceRange.layerCount = specification_.Layers;

    vulkan::InsertImageMemoryBarrier(
        commandBuffer, info_.Image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        subresourceRange);

    VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
  }

  UpdateDescriptor();
}

void VulkanImage2D::CreatePerLayerImageViews() {
  Ref<VulkanImage2D> instance = this;
  Renderer::Submit(
      [instance]() mutable { instance->RT_CreatePerLayerImageViews(); });
}

void VulkanImage2D::RT_CreatePerLayerImageViews() {
  BE_CORE_ASSERT(specification_.Layers > 1);

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

  VkImageAspectFlags aspectMask = Utils::IsDepthFormat(specification_.Format)
                                      ? VK_IMAGE_ASPECT_DEPTH_BIT
                                      : VK_IMAGE_ASPECT_COLOR_BIT;
  if (specification_.Format == ImageFormat::DEPTH24STENCIL8)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  const VkFormat vulkanFormat =
      Utils::VulkanImageFormat(specification_.Format);

  per_layer_image_views_.resize(specification_.Layers);
  for (uint32_t layer = 0; layer < specification_.Layers; layer++) {
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vulkanFormat;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = specification_.Mips;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.image = info_.Image;
    vkCreateImageView(device, &imageViewCreateInfo, nullptr,
                                      &per_layer_image_views_[layer]);
    vulkan::SetDebugUtilsObjectName(
        device, VK_OBJECT_TYPE_IMAGE_VIEW,
        fmt::format("{} image view layer: {}", specification_.DebugName,
                    layer),
        per_layer_image_views_[layer]);
  }
}

VkImageView VulkanImage2D::GetMipImageView(uint32_t mip) {
  if (per_mip_image_views_.find(mip) == per_mip_image_views_.end()) {
    Ref<VulkanImage2D> instance = this;
    Renderer::Submit(
        [instance, mip]() mutable { instance->RT_GetMipImageView(mip); });
    return VK_NULL_HANDLE;
  }

  return per_mip_image_views_.at(mip);
}

VkImageView VulkanImage2D::RT_GetMipImageView(const uint32_t mip) {
  auto it = per_mip_image_views_.find(mip);
  if (it != per_mip_image_views_.end()) return it->second;

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

  VkImageAspectFlags aspectMask = Utils::IsDepthFormat(specification_.Format)
                                      ? VK_IMAGE_ASPECT_DEPTH_BIT
                                      : VK_IMAGE_ASPECT_COLOR_BIT;
  if (specification_.Format == ImageFormat::DEPTH24STENCIL8)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  VkFormat vulkanFormat = Utils::VulkanImageFormat(specification_.Format);

  VkImageViewCreateInfo imageViewCreateInfo = {};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = vulkanFormat;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.subresourceRange = {};
  imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
  imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = 1;
  imageViewCreateInfo.image = info_.Image;

  vkCreateImageView(device, &imageViewCreateInfo, nullptr,
                                    &per_mip_image_views_[mip]);
  vulkan::SetDebugUtilsObjectName(
      device, VK_OBJECT_TYPE_IMAGE_VIEW,
      fmt::format("{} image view mip: {}", specification_.DebugName, mip),
      per_mip_image_views_[mip]);
  return per_mip_image_views_.at(mip);
}

void VulkanImage2D::RT_CreatePerSpecificLayerImageViews(
    const std::vector<uint32_t>& layerIndices) {
  BE_CORE_ASSERT(specification_.Layers > 1);

  VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

  VkImageAspectFlags aspectMask = Utils::IsDepthFormat(specification_.Format)
                                      ? VK_IMAGE_ASPECT_DEPTH_BIT
                                      : VK_IMAGE_ASPECT_COLOR_BIT;
  if (specification_.Format == ImageFormat::DEPTH24STENCIL8)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  const VkFormat vulkanFormat =
      Utils::VulkanImageFormat(specification_.Format);

  // BE_CORE_ASSERT(per_layer_image_views_.size() == specification_.Layers);
  if (per_layer_image_views_.empty())
    per_layer_image_views_.resize(specification_.Layers);

  for (uint32_t layer : layerIndices) {
    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vulkanFormat;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.subresourceRange = {};
    imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = specification_.Mips;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.image = info_.Image;
    vkCreateImageView(device, &imageViewCreateInfo, nullptr,
                                      &per_layer_image_views_[layer]);
    vulkan::SetDebugUtilsObjectName(
        device, VK_OBJECT_TYPE_IMAGE_VIEW,
        fmt::format("{} image view layer: {}", specification_.DebugName,
                    layer),
        per_layer_image_views_[layer]);
  }
}

void VulkanImage2D::UpdateDescriptor() {
  if (specification_.Format == ImageFormat::DEPTH24STENCIL8 ||
      specification_.Format == ImageFormat::DEPTH32F ||
      specification_.Format == ImageFormat::DEPTH32FSTENCIL8UINT)
    descriptor_image_info_.imageLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  else if (specification_.Usage == ImageUsage::Storage)
    descriptor_image_info_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  else
    descriptor_image_info_.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  if (specification_.Usage == ImageUsage::Storage)
    descriptor_image_info_.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
  else if (specification_.Usage == ImageUsage::HostRead)
    descriptor_image_info_.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

  descriptor_image_info_.imageView = info_.ImageView;
  descriptor_image_info_.sampler = info_.Sampler;

  // BE_CORE_WARN_TAG("Renderer", "VulkanImage2D::UpdateDescriptor to ImageView
  // = {0}", (const void*)info_.ImageView);
}

const std::map<VkImage, WeakRef<VulkanImage2D>>& VulkanImage2D::GetImageRefs() {
  return s_ImageReferences;
}

void VulkanImage2D::CopyToHostBuffer(Buffer& buffer) {
  auto device = VulkanContext::GetCurrentDevice();
  auto vulkanDevice = device->GetVulkanDevice();
  VulkanAllocator allocator("Image2D");

  uint64_t bufferSize = specification_.Width * specification_.Height *
                        Utils::GetImageFormatBPP(specification_.Format);

  // Create staging buffer
  VkBufferCreateInfo bufferCreateInfo{};
  bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCreateInfo.size = bufferSize;
  bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer stagingBuffer;
  VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(
      bufferCreateInfo, VMA_MEMORY_USAGE_GPU_TO_CPU, stagingBuffer);

  uint32_t mipCount = 1;
  uint32_t mipWidth = specification_.Width, mipHeight = specification_.Height;

  VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.levelCount = mipCount;
  subresourceRange.layerCount = 1;

  vulkan::InsertImageMemoryBarrier(
      copyCmd, info_.Image, VK_ACCESS_TRANSFER_READ_BIT, 0,
      descriptor_image_info_.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      subresourceRange);

  uint64_t mipDataOffset = 0;
  for (uint32_t mip = 0; mip < mipCount; mip++) {
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = mip;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = mipWidth;
    bufferCopyRegion.imageExtent.height = mipHeight;
    bufferCopyRegion.imageExtent.depth = 1;
    bufferCopyRegion.bufferOffset = mipDataOffset;

    vkCmdCopyImageToBuffer(copyCmd, info_.Image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer,
                           1, &bufferCopyRegion);

    uint64_t mipDataSize = mipWidth * mipHeight * sizeof(float) * 4 * 6;
    mipDataOffset += mipDataSize;
    mipWidth /= 2;
    mipHeight /= 2;
  }

  vulkan::InsertImageMemoryBarrier(
      copyCmd, info_.Image, VK_ACCESS_TRANSFER_READ_BIT, 0,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, descriptor_image_info_.imageLayout,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      subresourceRange);

  device->FlushCommandBuffer(copyCmd);

  // Copy data from staging buffer
  uint8_t* srcData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
  buffer.Allocate(bufferSize);
  memcpy(buffer.Data, srcData, bufferSize);
  allocator.UnmapMemory(stagingBufferAllocation);

  allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
}

VulkanImageView::VulkanImageView(const ImageViewSpecification& specification)
    : specification_(specification) {
  Invalidate();
}

VulkanImageView::~VulkanImageView() {
  Renderer::SubmitResourceFree([imageView = image_view_]() mutable {
    auto device = VulkanContext::GetCurrentDevice();
    VkDevice vulkanDevice = device->GetVulkanDevice();

    vkDestroyImageView(vulkanDevice, imageView, nullptr);
  });

  image_view_ = VK_NULL_HANDLE;
}

void VulkanImageView::Invalidate() {
  Ref<VulkanImageView> instance = this;
  Renderer::Submit([instance]() mutable { instance->RT_Invalidate(); });
}

void VulkanImageView::RT_Invalidate() {
  auto device = VulkanContext::GetCurrentDevice();
  VkDevice vulkanDevice = device->GetVulkanDevice();

  Ref<VulkanImage2D> vulkanImage = specification_.Image.As<VulkanImage2D>();
  const auto& imageSpec = vulkanImage->GetSpecification();

  VkImageAspectFlags aspectMask = Utils::IsDepthFormat(imageSpec.Format)
                                      ? VK_IMAGE_ASPECT_DEPTH_BIT
                                      : VK_IMAGE_ASPECT_COLOR_BIT;
  if (imageSpec.Format == ImageFormat::DEPTH24STENCIL8)
    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  VkFormat vulkanFormat = Utils::VulkanImageFormat(imageSpec.Format);

  VkImageViewCreateInfo imageViewCreateInfo = {};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.viewType = imageSpec.Layers > 1
                                     ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                     : VK_IMAGE_VIEW_TYPE_2D;
  imageViewCreateInfo.format = vulkanFormat;
  imageViewCreateInfo.flags = 0;
  imageViewCreateInfo.subresourceRange = {};
  imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
  imageViewCreateInfo.subresourceRange.baseMipLevel = specification_.Mip;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = imageSpec.Layers;
  imageViewCreateInfo.image = vulkanImage->GetImageInfo().Image;
  vkCreateImageView(vulkanDevice, &imageViewCreateInfo, nullptr,
                                    &image_view_);
  vulkan::SetDebugUtilsObjectName(
      vulkanDevice, VK_OBJECT_TYPE_IMAGE_VIEW,
      fmt::format("{} default image view", specification_.DebugName),
      image_view_);

  descriptor_image_info_ = vulkanImage->GetDescriptorInfoVulkan();
  descriptor_image_info_.imageView = image_view_;
}
}

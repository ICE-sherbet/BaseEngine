#include "VulkanTexture.h"

#include "RendererApi.h"
#include "TextureImporter.h"
#include "VulkanAPI.h"
#include "VulkanAllocator.h"
#include "VulkanUtilities.h"

namespace base_engine {

namespace Utils {

static VkSamplerAddressMode VulkanSamplerWrap(TextureWrap wrap) {
  switch (wrap) {
    case TextureWrap::Clamp:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureWrap::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  }
  BE_CORE_ASSERT(false, "Unknown wrap mode");
  return static_cast<VkSamplerAddressMode>(0);
}

static VkFilter VulkanSamplerFilter(TextureFilter filter) {
  switch (filter) {
    case TextureFilter::Linear:
      return VK_FILTER_LINEAR;
    case TextureFilter::Nearest:
      return VK_FILTER_NEAREST;
    case TextureFilter::Cubic:
      return VK_FILTER_CUBIC_IMG;
  }
  BE_CORE_ASSERT(false, "Unknown filter");
  return static_cast<VkFilter>(0);
}

static size_t GetMemorySize(ImageFormat format, uint32_t width,
                            uint32_t height) {
  switch (format) {
    case ImageFormat::RED16UI:
      return width * height * sizeof(uint16_t);
    case ImageFormat::RG16F:
      return width * height * 2 * sizeof(uint16_t);
    case ImageFormat::RG32F:
      return width * height * 2 * sizeof(float);
    case ImageFormat::RED32F:
      return width * height * sizeof(float);
    case ImageFormat::RED8UN:
      return width * height;
    case ImageFormat::RED8UI:
      return width * height;
    case ImageFormat::RGBA:
      return width * height * 4;
    case ImageFormat::RGBA32F:
      return width * height * 4 * sizeof(float);
    case ImageFormat::B10R11G11UF:
      return width * height * sizeof(float);
  }
  BE_CORE_ASSERT(false);
  return 0;
}

void SetImageLayout(
    VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
  VkImageMemoryBarrier imageMemoryBarrier = {};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.oldLayout = oldImageLayout;
  imageMemoryBarrier.newLayout = newImageLayout;
  imageMemoryBarrier.image = image;
  imageMemoryBarrier.subresourceRange = subresourceRange;

  switch (oldImageLayout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
      imageMemoryBarrier.srcAccessMask = 0;
      break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.srcAccessMask =
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      break;
  }

  switch (newImageLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask =
          imageMemoryBarrier.dstAccessMask |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      if (imageMemoryBarrier.srcAccessMask == 0) {
        imageMemoryBarrier.srcAccessMask =
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
      }
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    default:
      break;
  }

  vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0,
                       nullptr, 1, &imageMemoryBarrier);
}

static bool ValidateSpecification(const TextureSpecification& specification) {
  bool result = true;

  result = specification.Width > 0 && specification.Height > 0 &&
           specification.Width < 65536 && specification.Height < 65536;
  BE_CORE_VERIFY(result);

  return result;
}

}  // namespace Utils

VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification,
                                 const std::filesystem::path& filepath)
    : path_(filepath), specification_(specification) {
  Utils::ValidateSpecification(specification);

  image_data_ = TextureImporter::ToBufferFromFile(
      filepath, specification_.Format, specification_.Width,
      specification_.Height);
  if (!image_data_) {
    __debugbreak();
  }

  ImageSpecification imageSpec;
  imageSpec.Format = specification_.Format;
  imageSpec.Width = specification_.Width;
  imageSpec.Height = specification_.Height;
  imageSpec.Mips = specification.GenerateMips ? GetMipLevelCount() : 1;
  imageSpec.DebugName = specification.DebugName;
  imageSpec.CreateSampler = false;
  image_ = Image2D::Create(imageSpec);

  BE_CORE_ASSERT(specification_.Format != ImageFormat::None);

  Ref instance = this;
  Renderer::Submit([instance]() mutable { instance->Invalidate(); });
}

VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification,
                                 Buffer data)
    : specification_(specification) {
  if (specification_.Height == 0) {
    image_data_ = TextureImporter::ToBufferFromMemory(
        Buffer(data.Data, specification_.Width), specification_.Format,
        specification_.Width, specification_.Height);
    if (!image_data_) {
      // TODO(Yan): move this to asset manager
      image_data_ = TextureImporter::ToBufferFromFile(
          "Resources/Textures/ErrorTexture.png", specification_.Format,
          specification_.Width, specification_.Height);
    }

    Utils::ValidateSpecification(specification_);
  } else if (data) {
    Utils::ValidateSpecification(specification_);
    auto size = Utils::GetMemorySize(
        specification_.Format, specification_.Width, specification_.Height);
    image_data_ = Buffer::Copy(data.Data, size);
  } else {
    Utils::ValidateSpecification(specification_);
    auto size = Utils::GetMemorySize(
        specification_.Format, specification_.Width, specification_.Height);
    image_data_.Allocate(size);
    image_data_.ZeroInitialize();
  }

  ImageSpecification imageSpec;
  imageSpec.Format = specification_.Format;
  imageSpec.Width = specification_.Width;
  imageSpec.Height = specification_.Height;
  imageSpec.Mips =
      specification.GenerateMips ? VulkanTexture2D::GetMipLevelCount() : 1;
  imageSpec.DebugName = specification.DebugName;
  imageSpec.CreateSampler = false;
  if (specification.Storage) imageSpec.Usage = ImageUsage::Storage;
  image_ = Image2D::Create(imageSpec);

  Ref instance = this;
  Renderer::Submit([instance]() mutable { instance->Invalidate(); });
}

VulkanTexture2D::~VulkanTexture2D() {
  if (image_) image_->Release();

  image_data_.Release();
}

void VulkanTexture2D::Resize(const uint32_t width, const uint32_t height) {
  specification_.Width = width;
  specification_.Height = height;

  // Invalidate();

  Ref instance = this;
  Renderer::Submit([instance]() mutable { instance->Invalidate(); });
}

void VulkanTexture2D::Invalidate() {
  auto device = VulkanContext::GetCurrentDevice();
  auto vulkanDevice = device->GetVulkanDevice();

  image_->Release();

  uint32_t mipCount = specification_.GenerateMips ? GetMipLevelCount() : 1;

  ImageSpecification& imageSpec = image_->GetSpecification();
  imageSpec.Format = specification_.Format;
  imageSpec.Width = specification_.Width;
  imageSpec.Height = specification_.Height;
  imageSpec.Mips = mipCount;
  imageSpec.CreateSampler = false;
  if (!image_data_) imageSpec.Usage = ImageUsage::Storage;

  Ref<VulkanImage2D> image = image_.As<VulkanImage2D>();
  image->RT_Invalidate();

  auto& info = image->GetImageInfo();

  if (image_data_) {
    VkDeviceSize size = image_data_.Size;

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    VulkanAllocator allocator("Texture2D");

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(
        bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

    uint8_t* destData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
    BE_CORE_ASSERT(image_data_.Data);
    memcpy(destData, image_data_.Data, size);
    allocator.UnmapMemory(stagingBufferAllocation);

    VkCommandBuffer copyCmd = device->GetCommandBuffer(true);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = info.Image;
    imageMemoryBarrier.subresourceRange = subresourceRange;
    imageMemoryBarrier.srcAccessMask = 0;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_HOST_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &imageMemoryBarrier);

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = specification_.Width;
    bufferCopyRegion.imageExtent.height = specification_.Height;
    bufferCopyRegion.imageExtent.depth = 1;
    bufferCopyRegion.bufferOffset = 0;

    vkCmdCopyBufferToImage(copyCmd, stagingBuffer, info.Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &bufferCopyRegion);

    if (mipCount > 1) {
      vulkan::InsertImageMemoryBarrier(
          copyCmd, info.Image, VK_ACCESS_TRANSFER_WRITE_BIT,
          VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT, subresourceRange);
    } else {
      vulkan::InsertImageMemoryBarrier(
          copyCmd, info.Image, VK_ACCESS_TRANSFER_READ_BIT,
          VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          image->GetDescriptorInfoVulkan().imageLayout,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          subresourceRange);
    }

    device->FlushCommandBuffer(copyCmd);

    allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
  } else {
    VkCommandBuffer transitionCommandBuffer = device->GetCommandBuffer(true);
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = GetMipLevelCount();
    Utils::SetImageLayout(
        transitionCommandBuffer, info.Image, VK_IMAGE_LAYOUT_UNDEFINED,
        image->GetDescriptorInfoVulkan().imageLayout, subresourceRange);
    device->FlushCommandBuffer(transitionCommandBuffer);
  }

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.magFilter =
      Utils::VulkanSamplerFilter(specification_.SamplerFilter);
  samplerInfo.minFilter =
      Utils::VulkanSamplerFilter(specification_.SamplerFilter);
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU =
      Utils::VulkanSamplerWrap(specification_.SamplerWrap);
  samplerInfo.addressModeV =
      Utils::VulkanSamplerWrap(specification_.SamplerWrap);
  samplerInfo.addressModeW =
      Utils::VulkanSamplerWrap(specification_.SamplerWrap);
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = (float)mipCount;

  samplerInfo.maxAnisotropy = 1.0;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  info.Sampler = vulkan::CreateSampler(samplerInfo);
  image->UpdateDescriptor();

  if (!specification_.Storage) {
    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = Utils::VulkanImageFormat(specification_.Format);
    view.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                       VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};

    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.baseArrayLayer = 0;
    view.subresourceRange.layerCount = 1;
    view.subresourceRange.levelCount = mipCount;
    view.image = info.Image;

    vkCreateImageView(vulkanDevice, &view, nullptr, &info.ImageView);

    vulkan::SetDebugUtilsObjectName(
        vulkanDevice, VK_OBJECT_TYPE_IMAGE_VIEW,
        fmt::format("Texture view: {}", specification_.DebugName),
        info.ImageView);

    image->UpdateDescriptor();
  }

  if (image_data_ && specification_.GenerateMips && mipCount > 1)
    GenerateMips();

  image_data_.Release();
  image_data_ = Buffer();
}

void VulkanTexture2D::Bind(uint32_t slot) const {}

void VulkanTexture2D::Lock() {}

void VulkanTexture2D::Unlock() {}

Buffer VulkanTexture2D::GetWriteableBuffer() { return image_data_; }

const std::filesystem::path& VulkanTexture2D::GetPath() const { return path_; }

uint32_t VulkanTexture2D::GetMipLevelCount() const {
  return Utils::CalculateMipCount(specification_.Width,
                                  specification_.Height);
}

std::pair<uint32_t, uint32_t> VulkanTexture2D::GetMipSize(uint32_t mip) const {
  uint32_t width = specification_.Width;
  uint32_t height = specification_.Height;
  while (mip != 0) {
    width /= 2;
    height /= 2;
    mip--;
  }

  return {width, height};
}

void VulkanTexture2D::GenerateMips() {
  auto device = VulkanContext::GetCurrentDevice();

  Ref<VulkanImage2D> image = image_.As<VulkanImage2D>();
  const auto& info = image->GetImageInfo();

  const VkCommandBuffer blitCmd =
      VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

  const auto mipLevels = GetMipLevelCount();
  for (uint32_t i = 1; i < mipLevels; i++) {
    VkImageBlit imageBlit{};

    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel = i - 1;
    imageBlit.srcOffsets[1].x = int32_t(specification_.Width >> (i - 1));
    imageBlit.srcOffsets[1].y = int32_t(specification_.Height >> (i - 1));
    imageBlit.srcOffsets[1].z = 1;

    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel = i;
    imageBlit.dstOffsets[1].x = int32_t(specification_.Width >> i);
    imageBlit.dstOffsets[1].y = int32_t(specification_.Height >> i);
    imageBlit.dstOffsets[1].z = 1;

    VkImageSubresourceRange mipSubRange = {};
    mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mipSubRange.baseMipLevel = i;
    mipSubRange.levelCount = 1;
    mipSubRange.layerCount = 1;

    vulkan::InsertImageMemoryBarrier(
        blitCmd, info.Image, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    vkCmdBlitImage(blitCmd, info.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   info.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                   &imageBlit,
                   Utils::VulkanSamplerFilter(specification_.SamplerFilter));

    vulkan::InsertImageMemoryBarrier(
        blitCmd, info.Image, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange);
  }

  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  subresourceRange.layerCount = 1;
  subresourceRange.levelCount = mipLevels;

  vulkan::InsertImageMemoryBarrier(
      blitCmd, info.Image, VK_ACCESS_TRANSFER_READ_BIT,
      VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresourceRange);

  VulkanContext::GetCurrentDevice()->FlushCommandBuffer(blitCmd);
}

void VulkanTexture2D::CopyToHostBuffer(Buffer& buffer) {
  if (image_) image_.As<VulkanImage2D>()->CopyToHostBuffer(buffer);
}
}  // namespace base_engine

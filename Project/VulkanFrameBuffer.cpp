#include "VulkanFrameBuffer.h"

#include "Application.h"
#include "Platform/Windows/Window.h"
#include "RendererApi.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"
#include "VulkanImage.h"
#include "VulkanSwapChain.h"
#include "VulkanUtilities.h"

namespace base_engine {
namespace Utils {

inline VkAttachmentLoadOp GetVkAttachmentLoadOp(
    const FrameBufferSpecification& specification,
    const FrameBufferTextureSpecification& attachmentSpecification) {
  if (attachmentSpecification.LoadOp == AttachmentLoadOp::Inherit) {
    if (Utils::IsDepthFormat(attachmentSpecification.Format))
      return specification.ClearDepthOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                            : VK_ATTACHMENT_LOAD_OP_LOAD;

    return specification.ClearColorOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                          : VK_ATTACHMENT_LOAD_OP_LOAD;
  }

  return attachmentSpecification.LoadOp == AttachmentLoadOp::Clear
             ? VK_ATTACHMENT_LOAD_OP_CLEAR
             : VK_ATTACHMENT_LOAD_OP_LOAD;
}

}  // namespace Utils

VulkanFrameBuffer::VulkanFrameBuffer(
    const FrameBufferSpecification& specification)
    : specification_(specification) {
  if (specification.Width == 0) {
    width_ = Application::Get().GetWindow().GetWidth();
    height_ = Application::Get().GetWindow().GetHeight();
  } else {
    width_ =
        static_cast<uint32_t>(specification.Width * specification_.Scale);
    height_ =
        static_cast<uint32_t>(specification.Height * specification_.Scale);
  }

  if (!specification_.ExistingFrameBuffer) {
    uint32_t attachment_index = 0;
    for (const auto& attachment_spec :
         specification_.Attachments.Attachments) {
      if (specification_.ExistingImage) {
        if (Utils::IsDepthFormat(attachment_spec.Format))
          depth_attachment_image_ = specification_.ExistingImage;
        else
          attachment_images_.emplace_back(specification_.ExistingImage);
      } else if (specification_.ExistingImages.contains(attachment_index)) {
        if (Utils::IsDepthFormat(attachment_spec.Format))
          depth_attachment_image_ =
              specification_.ExistingImages.at(attachment_index);
        else
          attachment_images_.emplace_back();
      } else if (Utils::IsDepthFormat(attachment_spec.Format)) {
        ImageSpecification spec;
        spec.Format = attachment_spec.Format;
        spec.Usage = ImageUsage::Attachment;
        spec.Transfer = specification_.Transfer;
        spec.Width = static_cast<uint32_t>(width_ * specification_.Scale);
        spec.Height = static_cast<uint32_t>(height_ * specification_.Scale);
        spec.DebugName = fmt::format("{0}-DepthAttachment{1}",
                                     specification_.DebugName.empty()
                                         ? "Unnamed FB"
                                         : specification_.DebugName,
                                     attachment_index);
        depth_attachment_image_ = Image2D::Create(spec);
      } else {
        ImageSpecification spec;
        spec.Format = attachment_spec.Format;
        spec.Usage = ImageUsage::Attachment;
        spec.Transfer = specification_.Transfer;
        spec.Width = static_cast<uint32_t>(width_ * specification_.Scale);
        spec.Height = static_cast<uint32_t>(height_ * specification_.Scale);
        spec.DebugName = fmt::format("{0}-ColorAttachment{1}",
                                     specification_.DebugName.empty()
                                         ? "Unnamed FB"
                                         : specification_.DebugName,
                                     attachment_index);
        attachment_images_.emplace_back(Image2D::Create(spec));
      }
      attachment_index++;
    }
  }

  BE_CORE_ASSERT(specification.Attachments.Attachments.size());
  Resize(width_, height_, true);
}

VulkanFrameBuffer::~VulkanFrameBuffer() { Release(); }

void VulkanFrameBuffer::Release() {
  if (frame_buffer_) {
    VkFramebuffer framebuffer = frame_buffer_;
    Renderer::SubmitResourceFree([framebuffer]() {
      const auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
      vkDestroyFramebuffer(device, framebuffer, nullptr);
    });

    if (!specification_.ExistingFrameBuffer) {
      uint32_t attachmentIndex = 0;
      for (Ref<VulkanImage2D> image : attachment_images_) {
        if (specification_.ExistingImages.contains(attachmentIndex)) continue;

        if (image->GetSpecification().Layers == 1 ||
            attachmentIndex == 0 && !image->GetLayerImageView(0))
          image->Release();
        attachmentIndex++;
      }

      if (depth_attachment_image_) {
        if (!specification_.ExistingImages.contains(
                specification_.Attachments.Attachments.size() - 1))
          depth_attachment_image_->Release();
      }
    }
  }
}

void VulkanFrameBuffer::Resize(uint32_t width, uint32_t height,
                               const bool force_recreate) {
  if (!force_recreate && (width_ == width && height_ == height)) return;

  Ref instance = this;
  Renderer::Submit([instance, width, height]() mutable {
    instance->width_ =
        static_cast<uint32_t>(width * instance->specification_.Scale);
    instance->height_ =
        static_cast<uint32_t>(height * instance->specification_.Scale);
    if (!instance->specification_.SwapChainTarget) {
      instance->RT_Invalidate();
    } else {
      VulkanSwapChain& swap_chain =
          dynamic_cast<Window&>(Application::Get().GetWindow()).GetSwapChain();
      instance->render_pass_ = swap_chain.GetRenderPass();

      instance->clear_values_.clear();
      instance->clear_values_.emplace_back().color = {0.0f, 0.0f, 0.0f, 1.0f};
    }
  });

  for (auto& callback : resize_callbacks_) callback(this);
}

void VulkanFrameBuffer::AddResizeCallback(
    const std::function<void(Ref<FrameBuffer>)>& func) {
  resize_callbacks_.push_back(func);
}

void VulkanFrameBuffer::Invalidate() {
  Ref instance = this;
  Renderer::Submit([instance]() mutable { instance->RT_Invalidate(); });
}

void VulkanFrameBuffer::RT_Invalidate() {
  auto device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();

  Release();

  VulkanAllocator allocator("FrameBuffer");

  std::vector<VkAttachmentDescription> attachment_descriptions;

  std::vector<VkAttachmentReference> color_attachment_references;
  VkAttachmentReference depth_attachment_reference;

  clear_values_.resize(specification_.Attachments.Attachments.size());

  bool create_images = attachment_images_.empty();

  if (specification_.ExistingFrameBuffer) attachment_images_.clear();

  uint32_t attachment_index = 0;
  for (const auto& attachment_spec : specification_.Attachments.Attachments) {
    if (Utils::IsDepthFormat(attachment_spec.Format)) {
      if (specification_.ExistingImage) {
        depth_attachment_image_ = specification_.ExistingImage;
      } else if (specification_.ExistingFrameBuffer) {
        Ref<VulkanFrameBuffer> existing_frame_buffer =
            specification_.ExistingFrameBuffer.As<VulkanFrameBuffer>();
        depth_attachment_image_ = existing_frame_buffer->GetDepthImage();
      } else if (specification_.ExistingImages.contains(attachment_index)) {
        Ref<Image2D> existing_image =
            specification_.ExistingImages.at(attachment_index);
        depth_attachment_image_ = existing_image;
      } else {
        Ref<VulkanImage2D> depth_attachment_image =
            depth_attachment_image_.As<VulkanImage2D>();
        auto& spec = depth_attachment_image->GetSpecification();
        spec.Width = static_cast<uint32_t>(width_ * specification_.Scale);
        spec.Height = static_cast<uint32_t>(height_ * specification_.Scale);
        depth_attachment_image->RT_Invalidate();
      }

      VkAttachmentDescription& attachment_description =
          attachment_descriptions.emplace_back();
      attachment_description.flags = 0;
      attachment_description.format =
          Utils::VulkanImageFormat(attachment_spec.Format);
      attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment_description.loadOp =
          Utils::GetVkAttachmentLoadOp(specification_, attachment_spec);
      attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachment_description.initialLayout =
          attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR
              ? VK_IMAGE_LAYOUT_UNDEFINED
              : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      attachment_description.finalLayout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      attachment_description.finalLayout =
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      depth_attachment_reference = {
          attachment_index, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

      clear_values_[attachment_index].depthStencil = {
          specification_.DepthClearValue, 0};
    } else {
      Ref<VulkanImage2D> color_attachment;
      if (specification_.ExistingFrameBuffer) {
        Ref<VulkanFrameBuffer> existing_frame_buffer =
            specification_.ExistingFrameBuffer.As<VulkanFrameBuffer>();
        Ref<Image2D> existing_image =
            existing_frame_buffer->GetImage(attachment_index);
        color_attachment =
            attachment_images_.emplace_back(existing_image).As<VulkanImage2D>();
      } else if (specification_.ExistingImages.contains(attachment_index)) {
        Ref<Image2D> existing_image =
            specification_.ExistingImages[attachment_index];
        color_attachment = existing_image.As<VulkanImage2D>();
        attachment_images_[attachment_index] = existing_image;
      } else {
        if (create_images) {
          ImageSpecification spec;
          spec.Format = attachment_spec.Format;
          spec.Usage = ImageUsage::Attachment;
          spec.Transfer = specification_.Transfer;
          spec.Width = static_cast<uint32_t>(width_ * specification_.Scale);
          spec.Height = static_cast<uint32_t>(height_ * specification_.Scale);
          color_attachment =
              attachment_images_.emplace_back(Image2D::Create(spec))
                  .As<VulkanImage2D>();
          BE_CORE_VERIFY(false);

        } else {
          Ref<Image2D> image = attachment_images_[attachment_index];
          ImageSpecification& spec = image->GetSpecification();
          spec.Width = static_cast<uint32_t>(width_ * specification_.Scale);
          spec.Height = static_cast<uint32_t>(height_ * specification_.Scale);
          color_attachment = image.As<VulkanImage2D>();
          if (color_attachment->GetSpecification().Layers == 1)
            color_attachment->RT_Invalidate();
          else if (attachment_index == 0 &&
                   specification_.ExistingImageLayers[0] == 0) {
            color_attachment->RT_Invalidate();
            color_attachment->RT_CreatePerSpecificLayerImageViews(
                specification_.ExistingImageLayers);
          } else if (attachment_index == 0) {
            color_attachment->RT_CreatePerSpecificLayerImageViews(
                specification_.ExistingImageLayers);
          }
        }
      }

      auto& attachment_description = attachment_descriptions.emplace_back();
      attachment_description.flags = 0;
      attachment_description.format =
          Utils::VulkanImageFormat(attachment_spec.Format);
      attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
      attachment_description.loadOp =
          Utils::GetVkAttachmentLoadOp(specification_, attachment_spec);
      attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      attachment_description.initialLayout =
          attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR
              ? VK_IMAGE_LAYOUT_UNDEFINED
              : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      attachment_description.finalLayout =
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

      const auto& clear_color = specification_.ClearColor;
      clear_values_[attachment_index].color = {
          {clear_color.r, clear_color.g, clear_color.b, clear_color.a}};
      color_attachment_references.emplace_back(VkAttachmentReference{
          attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    }

    attachment_index++;
  }

  VkSubpassDescription subpass_description = {};
  subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass_description.colorAttachmentCount = color_attachment_references.size();
  subpass_description.pColorAttachments = color_attachment_references.data();
  if (depth_attachment_image_)
    subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

  std::vector<VkSubpassDependency> dependencies;

  if (!attachment_images_.empty()) {
    {
      VkSubpassDependency& depedency = dependencies.emplace_back();
      depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
      depedency.dstSubpass = 0;
      depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      depedency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      depedency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
    {
      VkSubpassDependency& depedency = dependencies.emplace_back();
      depedency.srcSubpass = 0;
      depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
      depedency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      depedency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
  }

  if (depth_attachment_image_) {
    {
      VkSubpassDependency& depedency = dependencies.emplace_back();
      depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
      depedency.dstSubpass = 0;
      depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      depedency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
      depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      depedency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    {
      VkSubpassDependency& depedency = dependencies.emplace_back();
      depedency.srcSubpass = 0;
      depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
      depedency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      depedency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
  }

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = attachment_descriptions.size();
  renderPassInfo.pAttachments = attachment_descriptions.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass_description;
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass_);
  vulkan::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_RENDER_PASS,
                                  specification_.DebugName, render_pass_);

  std::vector<VkImageView> attachments(attachment_images_.size());
  for (uint32_t i = 0; i < attachment_images_.size(); i++) {
    if (Ref<VulkanImage2D> image = attachment_images_[i].As<VulkanImage2D>();
        image->GetSpecification().Layers > 1)
      attachments[i] =
          image->GetLayerImageView(specification_.ExistingImageLayers[i]);
    else
      attachments[i] = image->GetImageInfo().ImageView;
    BE_CORE_ASSERT(attachments[i]);
  }

  if (depth_attachment_image_) {
    Ref<VulkanImage2D> image = depth_attachment_image_.As<VulkanImage2D>();
    if (specification_.ExistingImage && image->GetSpecification().Layers > 1) {
      BE_CORE_ASSERT(specification_.ExistingImageLayers.size() == 1,
                     "Depth attachments do not support deinterleaving");
      attachments.emplace_back(
          image->GetLayerImageView(specification_.ExistingImageLayers[0]));
    } else
      attachments.emplace_back(image->GetImageInfo().ImageView);

    BE_CORE_ASSERT(attachments.back());
  }

  VkFramebufferCreateInfo framebufferCreateInfo = {};
  framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferCreateInfo.renderPass = render_pass_;
  framebufferCreateInfo.attachmentCount = attachments.size();
  framebufferCreateInfo.pAttachments = attachments.data();
  framebufferCreateInfo.width = width_;
  framebufferCreateInfo.height = height_;
  framebufferCreateInfo.layers = 1;

  vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &frame_buffer_);
  vulkan::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_FRAMEBUFFER,
                                  specification_.DebugName, frame_buffer_);
}

}  // namespace base_engine

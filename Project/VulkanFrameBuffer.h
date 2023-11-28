// @VulkanFrameBuffer.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include <vulkan/vulkan_core.h>

#include "FrameBuffer.h"

namespace base_engine {
class VulkanFrameBuffer : public FrameBuffer {
 public:
	explicit VulkanFrameBuffer(const FrameBufferSpecification& spec);
  virtual ~VulkanFrameBuffer();

  void Resize(uint32_t width, uint32_t height,
              bool force_recreate = false) override;
  void AddResizeCallback(
      const std::function<void(Ref<FrameBuffer>)>& func) override;

  void BindTexture(uint32_t attachment_index = 0,
                   uint32_t slot = 0) const override {}

  uint32_t GetWidth() const override { return width_; }
  uint32_t GetHeight() const override { return height_; }

  Ref<Image2D> GetImage(const uint32_t attachment_index = 0) const override {
    BE_CORE_ASSERT(attachment_index < attachment_images_.size());
    return attachment_images_[attachment_index];
  }
  Ref<Image2D> GetDepthImage() const override {
    return depth_attachment_image_;
  }
  size_t GetColorAttachmentCount() const override {
    return specification_.SwapChainTarget ? 1 : attachment_images_.size();
  }
  bool HasDepthAttachment() const override {
    return (bool)depth_attachment_image_;
  }
  VkRenderPass GetRenderPass() const { return render_pass_; }
  VkFramebuffer GetVulkanFramebuffer() const { return frame_buffer_; }
  const std::vector<VkClearValue>& GetVulkanClearValues() const {
    return clear_values_;
  }

  const FrameBufferSpecification& GetSpecification() const override {
    return specification_;
  }

  void Invalidate();
  void RT_Invalidate();
  void Release();

 private:
  FrameBufferSpecification specification_;
  uint32_t width_ = 0, height_ = 0;

  std::vector<Ref<Image2D>> attachment_images_;
  Ref<Image2D> depth_attachment_image_;

  std::vector<VkClearValue> clear_values_;

  VkRenderPass render_pass_ = VK_NULL_HANDLE;
  VkFramebuffer frame_buffer_ = VK_NULL_HANDLE;

  std::vector<std::function<void(Ref<FrameBuffer>)>> resize_callbacks_;
};
}  // namespace base_engine

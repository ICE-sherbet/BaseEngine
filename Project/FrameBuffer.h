// @FrameBuffer.h
// @brief
// @author ICE
// @date 2023/11/24
//
// @details

#pragma once
#include <glm/vec4.hpp>

#include "Image.h"
#include "Ref.h"

namespace base_engine {

class FrameBuffer;

enum class FrameBufferBlendMode {
  None = 0,
  OneZero,
  SrcAlphaOneMinusSrcAlpha,
  Additive,
  Zero_SrcColor
};

enum class AttachmentLoadOp { Inherit = 0, Clear = 1, Load = 2 };

struct FrameBufferTextureSpecification {
  FrameBufferTextureSpecification() = default;
  FrameBufferTextureSpecification(ImageFormat format) : Format(format) {}

  ImageFormat Format;
  bool Blend = true;
  FrameBufferBlendMode BlendMode =
      FrameBufferBlendMode::SrcAlphaOneMinusSrcAlpha;
  AttachmentLoadOp LoadOp = AttachmentLoadOp::Inherit;
};

struct FrameBufferAttachmentSpecification {
  FrameBufferAttachmentSpecification() = default;
  FrameBufferAttachmentSpecification(
      const std::initializer_list<FrameBufferTextureSpecification>& attachments)
      : Attachments(attachments) {}

  std::vector<FrameBufferTextureSpecification> Attachments;
};

struct FrameBufferSpecification {
  float Scale = 1.0f;
  uint32_t Width = 0;
  uint32_t Height = 0;
  glm::vec4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
  float DepthClearValue = 0.0f;
  bool ClearColorOnLoad = true;
  bool ClearDepthOnLoad = true;

  FrameBufferAttachmentSpecification Attachments;
  uint32_t Samples = 1;

  bool NoResize = false;

  bool Blend = true;
  FrameBufferBlendMode BlendMode = FrameBufferBlendMode::None;

  bool SwapChainTarget = true;

  bool Transfer = false;

  Ref<Image2D> ExistingImage;
  std::vector<uint32_t> ExistingImageLayers;

  std::map<uint32_t, Ref<Image2D>> ExistingImages;

  Ref<FrameBuffer> ExistingFrameBuffer;

  std::string DebugName;
};

class FrameBuffer : public RefCounted {
 public:
  virtual void Resize(uint32_t width, uint32_t height,
                      bool force_recreate = false) = 0;
  virtual void AddResizeCallback(
      const std::function<void(Ref<FrameBuffer>)>& func) = 0;

  virtual void BindTexture(uint32_t attachment_index = 0,
                           uint32_t slot = 0) const = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  virtual Ref<Image2D> GetImage(uint32_t attachment_index = 0) const = 0;
  virtual size_t GetColorAttachmentCount() const = 0;
  virtual bool HasDepthAttachment() const = 0;
  virtual Ref<Image2D> GetDepthImage() const = 0;

  virtual const FrameBufferSpecification& GetSpecification() const = 0;

  static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
};
}  // namespace base_engine

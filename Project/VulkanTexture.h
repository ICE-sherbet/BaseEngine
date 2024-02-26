// @VulkanTexture.h
// @brief
// @author ICE
// @date 2023/11/14
//
// @details

#pragma once
#include "RendererTexture.h"
#include "VulkanImage.h"

namespace base_engine {

class VulkanTexture2D : public RendererTexture2D {
 public:
  VulkanTexture2D(const TextureSpecification& specification,
                  const std::filesystem::path& filepath);
  VulkanTexture2D(const TextureSpecification& specification,
                  Buffer data = Buffer());
  ~VulkanTexture2D() override;

  virtual void Resize(uint32_t width, uint32_t height) override;
  std::pair<uint32_t, uint32_t> GetSize() const override {
    return {specification_.Width, specification_.Height};
  }
  void Invalidate();

  virtual ImageFormat GetFormat() const override {
    return specification_.Format;
  }
  virtual uint32_t GetWidth() const override { return specification_.Width; }
  virtual uint32_t GetHeight() const override { return specification_.Height; }

  virtual void Bind(uint32_t slot = 0) const override;

  virtual Ref<Image2D> GetImage() const override { return image_; }
  virtual ResourceDescriptorInfo GetDescriptorInfo() const override {
    return image_.As<VulkanImage2D>()->GetDescriptorInfo();
  }
  const VkDescriptorImageInfo& GetDescriptorInfoVulkan() const {
    return *(VkDescriptorImageInfo*)GetDescriptorInfo();
  }

  void Lock() override;
  void Unlock() override;

  Buffer GetWriteableBuffer() override;
  bool Loaded() const override { return static_cast<bool>(image_data_); }
  const std::filesystem::path& GetPath() const override;
  uint32_t GetMipLevelCount() const override;
  virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const override;

  void GenerateMips();

  uint64_t GetHash() const override
  {
    return (uint64_t)
        image_.As<VulkanImage2D>()
                 ->GetDescriptorInfoVulkan()
                 .imageView;
  }

  void CopyToHostBuffer(Buffer& buffer);

 private:
  std::filesystem::path path_;
  TextureSpecification specification_;

  Buffer image_data_;

  Ref<Image2D> image_;
};
}  // namespace base_engine
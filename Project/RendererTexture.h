// @RendererTexture.h
// @brief
// @author ICE
// @date 2023/11/22
//
// @details

#pragma once
#include <filesystem>

#include "Asset.h"
#include "Buffer.h"
#include "Image.h"

namespace base_engine {

enum class TextureWrap { None = 0, Clamp, Repeat };

enum class TextureFilter { None = 0, Linear, Nearest, Cubic };

enum class TextureType { None = 0, Texture2D, TextureCube };

class Texture : public RendererResource {
 public:
  virtual ~Texture() {}

  virtual void Bind(uint32_t slot = 0) const = 0;

  virtual ImageFormat GetFormat() const = 0;
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;

  virtual uint32_t GetMipLevelCount() const = 0;
  virtual std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const = 0;

  virtual uint64_t GetHash() const = 0;

  virtual TextureType GetType() const = 0;
};

struct TextureSpecification {
  ImageFormat Format = ImageFormat::RGBA;
  uint32_t Width = 1;
  uint32_t Height = 1;
  TextureWrap SamplerWrap = TextureWrap::Repeat;
  TextureFilter SamplerFilter = TextureFilter::Linear;

  bool GenerateMips = true;
  bool SRGB = false;
  bool Storage = false;
  bool StoreLocally = false;

  std::string DebugName;
};

class RendererTexture2D : public Texture {
 public:
  static Ref<RendererTexture2D> Create(
      const TextureSpecification& specification);
  static Ref<RendererTexture2D> Create(
      const TextureSpecification& specification,
      const std::filesystem::path& filepath);
  static Ref<RendererTexture2D> Create(
      const TextureSpecification& specification, Buffer imageData);

  virtual void Resize(const uint32_t width, const uint32_t height) = 0;

  virtual Ref<Image2D> GetImage() const = 0;

  virtual void Lock() = 0;
  virtual void Unlock() = 0;

  virtual Buffer GetWriteableBuffer() = 0;

  virtual bool Loaded() const = 0;

  virtual const std::filesystem::path& GetPath() const = 0;

  virtual TextureType GetType() const override {
    return TextureType::Texture2D;
  }

  static AssetType GetStaticType() { return AssetType::kTexture; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }
};

}  // namespace base_engine

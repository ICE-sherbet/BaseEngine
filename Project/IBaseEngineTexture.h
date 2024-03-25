#pragma once
#if defined(__MOF__)
#include <Graphics/Texture.h>
#endif

#include <string>

namespace base_engine {
struct TextureHandle {
  size_t handle;
};

#if defined(__MOF__)
using TexturePtr = Mof::LPTexture;
#else
using TexturePtr = TextureHandle;
#endif

class IBaseEngineTexture {
 public:
  static IBaseEngineTexture* Create();

  virtual ~IBaseEngineTexture();

  virtual bool Load(std::string_view name) = 0;
  virtual TexturePtr Get(std::string_view name) = 0;
  virtual bool Release(std::string_view name) = 0;
  virtual void Clear() = 0;
};
}  // namespace base_engine
#include "Texture.h"

#include "MofTexture.h"

namespace base_engine {
Ref<Texture> TextureUtility::Create(const std::filesystem::path& path) {
  Ref<Texture> result;

#if defined(__MOF__)
  result = new MofTexture(path.string());
#else

#endif
  return result;
}

std::string TextureSerializer::GetAssetType(const std::filesystem::path& path) const
{
	if (path.extension() == ".png")
  {
    return "Texture";
  }
  return "";
}
}  // namespace base_engine

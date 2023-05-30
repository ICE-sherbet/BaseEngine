#include "Texture.h"

#include "MofTexture.h"

namespace base_engine {
Ref<Texture> TextureUtility::Create(const std::filesystem::path& path) {
  const auto result = new MofTexture(path.string());
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

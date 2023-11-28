#include "TextureImporter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace base_engine {

Buffer TextureImporter::ToBufferFromFile(const std::filesystem::path& path,
                                         ImageFormat& out_format,
                                         uint32_t& out_width,
                                         uint32_t& out_height) {
  Buffer image_buffer;
  const std::string path_string = path.string();

  int width, height, channels;
  if (stbi_is_hdr(path_string.c_str())) {
    image_buffer.Data = reinterpret_cast<uint8_t*>(
        stbi_loadf(path_string.c_str(), &width, &height, &channels, 4));
    image_buffer.Size = width * height * 4 * sizeof(float);
    out_format = ImageFormat::RGBA32F;
  } else {
    image_buffer.Data =
        stbi_load(path_string.c_str(), &width, &height, &channels, 4);
    image_buffer.Size = width * height * 4;
    out_format = ImageFormat::RGBA;
  }

  if (!image_buffer.Data) return {};

  out_width = width;
  out_height = height;
  return image_buffer;
}

Buffer TextureImporter::ToBufferFromMemory(const Buffer& buffer,
                                           ImageFormat& out_format,
                                           uint32_t& out_width,
                                           uint32_t& out_height) {
  Buffer image_buffer;

  int width, height, channels;
  if (stbi_is_hdr_from_memory(static_cast<const stbi_uc*>(buffer.Data),
                              static_cast<int>(buffer.Size))) {
    image_buffer.Data = reinterpret_cast<uint8_t*>(stbi_loadf_from_memory(
        static_cast<const stbi_uc*>(buffer.Data), static_cast<int>(buffer.Size),
        &width, &height, &channels, STBI_rgb_alpha));
    image_buffer.Size = width * height * 4 * sizeof(float);
    out_format = ImageFormat::RGBA32F;
  } else {
    image_buffer.Data = stbi_load_from_memory(
        static_cast<const stbi_uc*>(buffer.Data), static_cast<int>(buffer.Size),
        &width, &height, &channels, STBI_rgb_alpha);
    image_buffer.Size = width * height * 4;
    out_format = ImageFormat::RGBA;
  }

  if (!image_buffer.Data) return {};

  out_width = width;
  out_height = height;
  return image_buffer;
}
}  // namespace base_engine
// @TextureImporter.h
// @brief
// @author ICE
// @date 2023/11/22
// 
// @details

#pragma once
#include <cstdint>
#include <filesystem>

#include "Image.h"

namespace base_engine
{
class TextureImporter {
 public:
  static Buffer ToBufferFromFile(const std::filesystem::path& path,
                                 ImageFormat& out_format, uint32_t& out_width,
                                 uint32_t& out_height);
  static Buffer ToBufferFromMemory(const Buffer& buffer, ImageFormat& out_format,
                                   uint32_t& out_width, uint32_t& out_height);

};
}

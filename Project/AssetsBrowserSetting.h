// @AssetsBrowserSetting.h
// @brief
// @author ICE
// @date 2023/06/14
//
// @details

#pragma once
namespace base_engine::editor {
struct AssetsBrowserSetting {
  float thumbnail_size = 128;

  static AssetsBrowserSetting& Get();
};
}  // namespace base_engine::editor
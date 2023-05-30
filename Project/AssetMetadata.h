// @AssetMetadata.h
// @brief
// @author ICE
// @date 2023/04/04
//
// @details

#pragma once
#include <filesystem>

#include "Asset.h"

namespace base_engine {
struct AssetMetadata {
  AssetHandle handle = 0;
  AssetType type = AssetType::kNone;

  std::filesystem::path file_path = "";
  bool is_data_loaded = false;
  bool is_memory_asset = false;
  

  [[nodiscard]] bool IsValid() const { return handle != 0 && !is_memory_asset; }
};
}  // namespace base_engine

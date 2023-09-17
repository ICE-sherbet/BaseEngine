// @FileDialog.h
// @brief
// @author ICE
// @date 2023/07/12
//
// @details

#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace base_engine {
class FileDialog {
  FileDialog() = delete;
  ~FileDialog() = delete;

 public:
  static void SaveFile(const std::vector<std::string>& filters = {},
                       const std::filesystem::path& default_directory = U"",
                       std::string_view title = "");
};

}  // namespace base_engine

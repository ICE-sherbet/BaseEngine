// @FileSystem.h
// @brief
// @author ICE
// @date 2023/06/23
//
// @details

#pragma once
#include <filesystem>
#include <functional>
#include <string>

namespace base_engine {

enum class FileSystemAction { kAdded, kRename, kModified, kDelete };

struct FileSystemChangedEvent {
  FileSystemAction action;
  std::filesystem::path filepath;
  bool is_directory;

  std::wstring old_name;
};

class FileSystem {
 public:
  using FileSystemChangedCallbackFunc =
      std::function<void(const std::vector<FileSystemChangedEvent>&)>;

  static void AddFileSystemChangedCallback(
      const FileSystemChangedCallbackFunc& callback);
  static void ClearFileSystemChangedCallbacks();
  static void StartWatching();
  static void StopWatching();

 private:
  static unsigned long Watch();

  static std::vector<FileSystemChangedCallbackFunc> Callbacks;
};
}  // namespace base_engine

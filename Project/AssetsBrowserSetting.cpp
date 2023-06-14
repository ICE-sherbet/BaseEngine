#include "AssetsBrowserSetting.h"
namespace base_engine::editor {
AssetsBrowserSetting& AssetsBrowserSetting::Get() {
  static AssetsBrowserSetting settings;
  return settings;
}
}  // namespace base_engine::editor

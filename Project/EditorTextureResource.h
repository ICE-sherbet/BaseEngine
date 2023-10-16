// @EditorTextureResource.h
// @brief
// @author ICE
// @date 2023/06/12
//
// @details

#pragma once
#include "Ref.h"
#include "Texture.h"

namespace base_engine::editor {

class EditorTextureResource {
 public:
  void Initialize();
  Ref<Texture> GetIcon(const std::string& name);

 private:
  using IconMap = std::unordered_map<std::string, Ref<Texture>>;
  IconMap icon_map_;
};
class EditorColorResource {
 public:
  void Initialize();
  uint32_t GetColor(const std::string& name);

private:
  using ColorMap = std::unordered_map<std::string, uint32_t>;
  ColorMap color_map_;
};
;

class ThemeDB {
 public:
  static ThemeDB* GetInstance() {
    if (!instance_) {
      instance_ = new ThemeDB;
    }
    return instance_;
  }
  static void Destroy() {
    if (!instance_) return;
    delete instance_;
    instance_ = nullptr;
  }
  Ref<Texture> GetIcon(const std::string& name);
  uint32_t GetColor(const std::string& name);

 private:
  static ThemeDB* instance_;
  ThemeDB();
  EditorTextureResource icon_resource_{};
  EditorColorResource color_resource_{};
};
}  // namespace base_engine::editor

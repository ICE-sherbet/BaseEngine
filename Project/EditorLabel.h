// @EditorLabel.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include <string>

#include "EditorControl.h"

namespace base_engine::editor {
class EditorLabel : public EditorControl {
 public:
  explicit EditorLabel(const std::string& text = "") : text_(text) {}
  void Notification(int type) override;

  [[nodiscard]] std::string Text() const { return text_; }
  void SetText(const std::string& text) { text_ = text; }

 private:
  std::string text_;
};
}  // namespace base_engine::editor

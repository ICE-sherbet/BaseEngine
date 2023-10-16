// @EditorTextBox.h
// @brief
// @author ICE
// @date 2023/07/08
//
// @details

#pragma once

#include "EditorControl.h"

namespace base_engine::editor {

class EditorTextBox : public EditorControl {
 public:
  explicit EditorTextBox(const std::string& text = "") : text_(text) {}

  [[nodiscard]] std::string Text() const { return text_; }
  void SetText(const std::string& text) { text_ = text; }

  void Notification(int type) override;

 private:
  void TextChanged();

  std::string text_;
};
}  // namespace base_engine::editor

// @EditorButton.h
// @brief
// @author ICE
// @date 2023/06/29
//
// @details

#pragma once
#include "EditorControl.h"

namespace base_engine::editor {

class EditorButton : public EditorControl {
 public:
  explicit EditorButton(const std::string& text = "") : text_(text) {}

  [[nodiscard]] std::string Text() const { return text_; }
  void SetText(const std::string& text) { text_ = text; }

  void Notification(int type) override;

 private:
  void DrawButton();
  void Pressed();
  std::string text_;
};
}  // namespace base_engine::editor

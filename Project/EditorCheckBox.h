// @EditorCheckBox.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "EditorControl.h"

namespace base_engine::editor {
class EditorCheckBox : public EditorControl {
 public:
  EditorCheckBox(const std::string& text = "", const bool value = false);

  void SetPressed(bool v);
  bool IsPressed() const;

  void Notification(int type) override;

  [[nodiscard]] std::string Text() const;
  void SetText(const std::string& text);

private:
  std::string text_;
  bool value_ = false;
};
}  // namespace base_engine::editor

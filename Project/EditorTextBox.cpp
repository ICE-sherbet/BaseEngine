#include "EditorTextBox.h"
#include "ImGuiUtilities.h"

#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
namespace base_engine::editor {
void EditorTextBox::Notification(int type) {
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      edited = ImGui::InputText(ui::GenerateID(), &text_);
      break;
    }
  }

  if (edited) {
    TextChanged();
  }
}

void EditorTextBox::TextChanged() { EmitSignal("TextChanged", text_); }
}  // namespace base_engine::editor

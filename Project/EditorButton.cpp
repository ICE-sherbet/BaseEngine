#include "EditorButton.h"

#include <imgui.h>

#include "ImGuiUtilities.h"
namespace base_engine::editor {
void EditorButton::Notification(int type) {
  switch (type) {
    case kControlDraw: {
      DrawButton();
    } break;
  }
}

void EditorButton::DrawButton() {
  if (ImGui::Button(ui::GenerateLabelID(text_))) {
    Pressed();
  }
}

void EditorButton::Pressed() { EmitSignal("Pressed"); }
}  // namespace base_engine::editor

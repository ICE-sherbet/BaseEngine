#include "EditorLabel.h"

#include "imgui/imgui.h"

namespace base_engine::editor {
void EditorLabel::Notification(int type) {
  switch (type) {
    case kControlDraw:
      ImGui::Text(text_.c_str());
      break;
  }
}
}  // namespace base_engine::editor

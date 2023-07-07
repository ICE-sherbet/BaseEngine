#include "EditorDragValueInt.h"

#include "ImGuiUtilities.h"

namespace base_engine::editor {
void EditorDragValueInt::Notification(const int type)
{
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      ImGui::SetNextItemWidth(100);
      edited =
          ImGui::DragFloat(ui::GenerateLabelID(text_), &value_, 1, min_, max_);
      break;
    }
  }

  if (edited) {
    EmitSignal("ValueChanged", value_);
  }
}

void EditorDragValueInt::Setup(int64_t min, int64_t max, int64_t step)
{
  min_ = min;
  max_ = max;
  
}
}  // namespace base_engine::editor

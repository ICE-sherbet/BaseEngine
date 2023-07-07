﻿#include "EditorDragValueFloat.h"

#include "imgui.h"
#include "ImGuiUtilities.h"

namespace base_engine::editor {
void EditorDragValueFloat::Notification(int type) {
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      ImGui::SetNextItemWidth(100);
      edited =
          ImGui::DragFloat(ui::GenerateLabelID(text_), &value_, 1, min_, max_);
      break;
    }
  }

  if(edited)
  {
    EmitSignal("ValueChanged", value_);
  }
}


void EditorDragValueFloat2::Notification(int type) {
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      edited = ImGui::DragFloat2(ui::GenerateLabelID(text_), value_.fv, 1, min_,
                                 max_);
      break;
    }
  }
}

void EditorDragValueFloat3::Notification(int type) {
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      edited = ImGui::DragFloat3(ui::GenerateLabelID(text_), value_.fv, 1, min_,
                                 max_);
      break;
    }
  }
}
void EditorDragValueFloat4::Notification(int type) {
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      edited = ImGui::DragFloat4(ui::GenerateLabelID(text_), value_.fv, 1, min_,
                                 max_);
      break;
    }
  }
}
}  // namespace base_engine::editor
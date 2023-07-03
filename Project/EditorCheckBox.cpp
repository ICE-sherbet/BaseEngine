#include "EditorCheckBox.h"

#include "imgui.h"
#include "ImGuiUtilities.h"

namespace base_engine::editor
{
EditorCheckBox::EditorCheckBox(const std::string& text, const bool value): text_(text), value_(value)
{}

void EditorCheckBox::SetPressed(bool v)
{
	value_ = v;
  EmitSignal("Pressed", value_);
}

bool EditorCheckBox::IsPressed() const
{ return value_; }

std::string EditorCheckBox::Text() const
{ return text_; }

void EditorCheckBox::SetText(const std::string& text)
{ text_ = text; }

void EditorCheckBox::Notification(int type)
{
  bool edited = false;
  switch (type) {
    case kControlDraw: {
      edited = ImGui::Checkbox(ui::GenerateLabelID(text_), &value_);
      break;
    }
  }

  if (edited)
  {
    SetPressed(value_);
  }
}
}

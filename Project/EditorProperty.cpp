#include "EditorProperty.h"

#include "imgui.h"

namespace base_engine::editor {
void EditorProperty::SetPropertyName(const std::string& name) {
  property_name_ = name;
  label_->SetText(property_name_);
}

void EditorProperty::SetObjectAndClassName(ObjectEntity& object,
                                           const std::string& class_name) {
  object_ = object;
  class_name_ = class_name;
}

void EditorProperty::Draw() const {
  label_->Notification(kControlDraw);
  ImGui::SameLine();
  for (const auto& control : controls_) {
    control->Notification(kControlDraw);
  }
}

EditorProperty::EditorProperty() { label_ = std::make_shared<EditorLabel>(); }

void EditorProperty::AddControl(std::shared_ptr<EditorControl> control) {
  controls_.emplace_back(control);
}

std::string EditorProperty::GetPropertyName() { return property_name_; }

void EditorProperty::EmitChanged(const std::string& class_name,
                                 const std::string& property,
                                 const Variant& value) {
  EmitSignal("PropertyChanged", class_name, property, value);
}

void EditorProperty::UpdateProperty() {}
}  // namespace base_engine::editor

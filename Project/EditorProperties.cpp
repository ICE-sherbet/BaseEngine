#include "EditorProperties.h"

#include <imgui/imgui.h>

#include "ComponentDB.h"

namespace base_engine::editor {
EditorPropertyCheck::EditorPropertyCheck() {
  checkbox_ = std::make_shared<EditorCheckBox>();
  checkbox_->Connect("pressed",
                     make_callable_function_pointer(
                         this, &EditorPropertyCheck::CheckBoxPressed));
}

void EditorPropertyCheck::CheckBoxPressed() {
  EmitChanged(class_name_, GetPropertyName(),
              static_cast<Variant>(checkbox_->IsPressed()));
}

void EditorPropertyCheck::UpdateProperty() {
  Variant r_value;
  ComponentDB::TryGetProperty(object_, class_name_, property_name_, r_value);
  checkbox_->SetPressed(static_cast<bool>(r_value));
}

EditorPropertyVector2::EditorPropertyVector2() {
  drag_values_[0] = std::make_shared<EditorDragValue>("x");
  drag_values_[1] = std::make_shared<EditorDragValue>("y");

  for (const auto& control : drag_values_) {
    control->Connect("ValueChanged",
                     make_callable_function_pointer(
                         this, &EditorPropertyVector2::ValueChanged));

    AddControl(control);
  }
}

void EditorPropertyVector2::UpdateProperty() {
  Variant r_value;
  ComponentDB::TryGetProperty(object_, class_name_, property_name_, r_value);
  const auto v = static_cast<Vector2>(r_value);
  drag_values_[0]->SetValue(v.x);
  drag_values_[1]->SetValue(v.y);
}

void EditorPropertyVector2::ValueChanged() {
  Vector3 v2;
  v2.x = drag_values_[0]->Value();
  v2.y = drag_values_[1]->Value();

  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(v2));
}

void EditorPropertyVector2::Draw() const {
  label_->Notification(kControlDraw);
  const auto width = std::min(ImGui::GetColumnWidth() / 3, 100.0f);

  ImGui::SetNextItemWidth(width);
  drag_values_[0]->Notification(kControlDraw);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(width);
  drag_values_[1]->Notification(kControlDraw);
}

EditorPropertyVector3::EditorPropertyVector3() {
  drag_values_[0] = std::make_shared<EditorDragValue>("x");
  drag_values_[1] = std::make_shared<EditorDragValue>("y");
  drag_values_[2] = std::make_shared<EditorDragValue>("z");

  for (const auto& control : drag_values_) {
    control->Connect("ValueChanged",
                     make_callable_function_pointer(
                         this, &EditorPropertyVector3::ValueChanged));

    AddControl(control);
  }
}

void EditorPropertyVector3::UpdateProperty() {
  Variant r_value;
  ComponentDB::TryGetProperty(object_, class_name_, property_name_, r_value);
  const auto v = static_cast<Vector3>(r_value);
  drag_values_[0]->SetValue(v.x);
  drag_values_[1]->SetValue(v.y);
  drag_values_[2]->SetValue(v.z);
}

void EditorPropertyVector3::ValueChanged(float dummy) {
  Vector3 v3;
  v3.x = drag_values_[0]->Value();
  v3.y = drag_values_[1]->Value();
  v3.z = drag_values_[2]->Value();

  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(v3));
}

void EditorPropertyVector3::Draw() const {
  label_->Notification(kControlDraw);
  const auto width = std::min(ImGui::GetColumnWidth() / 3, 100.0f);

  ImGui::SetNextItemWidth(width);
  drag_values_[0]->Notification(kControlDraw);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(width);
  drag_values_[1]->Notification(kControlDraw);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(width);
  drag_values_[2]->Notification(kControlDraw);
}

EditorPropertyAsset::EditorPropertyAsset() {
  asset_picker_ = std::make_shared<EditorAssetPicker>();
  asset_picker_->Connect("AssetSelected",
                         make_callable_function_pointer(
                             this, &EditorPropertyAsset::AssetSelected));
  AddControl(asset_picker_);
}

void EditorPropertyAsset::Initialize(const std::string& type) const {
  asset_picker_->SetSearchType(type);
}

void EditorPropertyAsset::Draw() const { EditorProperty::Draw(); }

void EditorPropertyAsset::UpdateProperty() {
  Variant r_value;
  ComponentDB::TryGetProperty(object_, class_name_, property_name_, r_value);
  const auto v = static_cast<AssetHandle>(r_value);
  asset_picker_->SetEditedAsset(v);
}

void EditorPropertyAsset::AssetSelected(AssetHandle handle) {
  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(handle));
}
}  // namespace base_engine::editor

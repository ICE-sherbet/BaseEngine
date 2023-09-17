#include "EditorProperties.h"

#include <imgui/imgui.h>

#include "ComponentDB.h"
#include "EditorTextBox.h"
#include "ImGuiUtilities.h"
#include "ObjectEntity.h"

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
  object_.TryGetProperty(class_name_, property_name_, r_value);
  checkbox_->SetPressed(static_cast<bool>(r_value));
}

EditorPropertyInteger::EditorPropertyInteger() {
  drag_value_ = std::make_shared<EditorDragValueInt>();
  drag_value_->Connect("ValueChanged",
                       make_callable_function_pointer(
                           this, &EditorPropertyInteger::ValueChanged));

  AddControl(drag_value_);
}

void EditorPropertyInteger::Draw() const { EditorProperty::Draw(); }

void EditorPropertyInteger::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<int>(r_value);
  drag_value_->SetValue(v);
}

void EditorPropertyInteger::Setup(int64_t min, int64_t max, int64_t step) {
  drag_value_->Setup(min, max, step);
}

void EditorPropertyInteger::ValueChanged() {
  int v = drag_value_->Value();

  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(v));
}

EditorPropertyFloat::EditorPropertyFloat() {
  drag_value_ = std::make_shared<EditorDragValueFloat>();
  drag_value_->Connect(
      "ValueChanged",
      make_callable_function_pointer(this, &EditorPropertyFloat::ValueChanged));

  AddControl(drag_value_);
}

void EditorPropertyFloat::Draw() const { EditorProperty::Draw(); }

void EditorPropertyFloat::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<float>(r_value);
  drag_value_->SetValue(v);
}
void EditorPropertyFloat::ValueChanged() {
  float v = drag_value_->Value();

  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(v));
}

EditorPropertyVector2::EditorPropertyVector2() {
  drag_values_[0] = std::make_shared<EditorDragValueFloat>("x");
  drag_values_[1] = std::make_shared<EditorDragValueFloat>("y");

  for (const auto& control : drag_values_) {
    control->Connect("ValueChanged",
                     make_callable_function_pointer(
                         this, &EditorPropertyVector2::ValueChanged));

    AddControl(control);
  }
}

void EditorPropertyVector2::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<Vector2>(r_value);
  drag_values_[0]->SetValue(v.x);
  drag_values_[1]->SetValue(v.y);
}

void EditorPropertyVector2::ValueChanged() {
  Vector2 v2;
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
  drag_values_[0] = std::make_shared<EditorDragValueFloat>("x");
  drag_values_[1] = std::make_shared<EditorDragValueFloat>("y");
  drag_values_[2] = std::make_shared<EditorDragValueFloat>("z");

  for (const auto& control : drag_values_) {
    control->Connect("ValueChanged",
                     make_callable_function_pointer(
                         this, &EditorPropertyVector3::ValueChanged));

    AddControl(control);
  }
}

void EditorPropertyVector3::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

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

EditorPropertyText::EditorPropertyText() {
  text_box_ = std::make_shared<EditorTextBox>();
  text_box_->Connect(
      "TextChanged",
      make_callable_function_pointer(this, &EditorPropertyText::TextChanged));

  AddControl(text_box_);
}

void EditorPropertyText::Draw() const { EditorProperty::Draw(); }

void EditorPropertyText::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<std::string>(r_value);
  text_box_->SetText(v);
}

void EditorPropertyText::TextChanged(const std::string& text) {
  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(text));
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
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<AssetHandle>(r_value);
  asset_picker_->SetEditedAsset(v);
}

void EditorPropertyAsset::AssetSelected(AssetHandle handle) {
  EmitChanged(class_name_, GetPropertyName(), static_cast<Variant>(handle));
}

class EditorPropertyLayers::EditorPropertyLayersCombo : public EditorControl {
 public:
  void Notification(int type) override {
    switch (type) {
      case kControlDraw: {
        DrawCombo();
      } break;
    }
  }

  [[nodiscard]] uint32_t Value() const { return value_; }

  void SetValue(const uint32_t value) { value_ = value; }

 private:
  void DrawCombo() {
    const auto is_single = std::has_single_bit(value_);
    const auto max_bit = std::bit_floor(value_);
    const auto max_bit_pos = std::countr_zero(max_bit);
    bool active = false;
    std::string preview =
        ((is_single) ? std::to_string(max_bit_pos + 1) : "Multi Select");
    if ((value_) == 0) preview = "None";
    if (ImGui::BeginCombo(ui::GenerateLabelID(text_), preview.c_str())) {
      for (int n = 0; n < 32; n++) {
        if (n == 0) {
          const bool is_selected = (value_) == 0;
          if (ImGui::Selectable(ui::GenerateLabelID("None"), is_selected)) {
            value_ = 0;
            active = true;
          }
          continue;
        }

        const bool is_selected = ((value_) & (1 << (n - 1)));
        if (ImGui::Selectable(ui::GenerateLabelID(std::to_string(n)),
                              is_selected)) {
          if (is_selected) {
            value_ &= ~(1 << (n - 1));
          } else {
            value_ |= (1 << (n - 1));
          }
          active = true;
        }
      }
      ImGui::EndCombo();
    }
    if (active) EmitSignal("ValueChanged");
  }

  std::string text_;
  uint32_t value_ = 0;
};

EditorPropertyLayers::EditorPropertyLayers() {
  combo_box_ = std::make_shared<EditorPropertyLayersCombo>();
  AddControl(combo_box_);
  combo_box_->Connect("ValueChanged",
                      make_callable_function_pointer(
                          this, &EditorPropertyLayers::ValueChanged));
}

EditorPropertyLayers::~EditorPropertyLayers() {}

void EditorPropertyLayers::Draw() const { EditorProperty::Draw(); }

void EditorPropertyLayers::UpdateProperty() {
  Variant r_value;
  object_.TryGetProperty(class_name_, property_name_, r_value);

  const auto v = static_cast<uint32_t>(r_value);
  combo_box_->SetValue(v);
}

void EditorPropertyLayers::ValueChanged() {
  EmitChanged(class_name_, GetPropertyName(),
              static_cast<Variant>(combo_box_->Value()));
}
}  // namespace base_engine::editor

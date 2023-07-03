﻿#include "EditorAssetPicker.h"

#include "AssetManager.h"
#include "ImGuiUtilities.h"
#include "imgui.h"

namespace base_engine::editor {
EditorAssetPicker::EditorAssetPicker(const std::string& text,
                                     const std::string& type)
    : text_(text), search_type_(type) {
  label_ = std::make_shared<EditorLabel>(text_);

  edit_button_ = std::make_shared<EditorButton>();
  edit_button_->SetText("None");

  edit_button_->Connect("Pressed", make_callable_function_pointer(
                                       this, &EditorAssetPicker::UpdateMenu));

  assign_button_ = std::make_shared<EditorButton>();
  assign_button_->SetText("OK");
  assign_button_->Connect(
      "Pressed",
      make_callable_function_pointer(this, &EditorAssetPicker::AssetSelect));

  cancel_button_ = std::make_shared<EditorButton>();
  cancel_button_->SetText("Cancel");
  cancel_button_->Connect("Pressed", make_callable_function_pointer(
                                         this, &EditorAssetPicker::Cancel));
}

void EditorAssetPicker::Notification(int type) {
  switch (type) {
    case kControlDraw: {
      Draw();
    } break;
  }
}

void EditorAssetPicker::SetEditedAsset(AssetHandle handle) {
  pick_ = handle;
  EditedAssetUpdate();
}

void EditorAssetPicker::UpdateMenu() { Popup(); }

void EditorAssetPicker::Popup() {
  show_popup_ = true;

  const std::string popup_name = "AssetSearchPopup";
  ImGui::OpenPopup(popup_name.c_str());
}

void EditorAssetPicker::Draw() {
  label_->Notification(kControlDraw);
  ImGui::SameLine();
  edit_button_->Notification(kControlDraw);
  PopupDraw();
}

void EditorAssetPicker::PopupDraw() {
  const std::string popup_name = "AssetSearchPopup";

  const auto& registry = AssetManager::GetAssetRegistry();

  if (ImGui::BeginPopup(popup_name.c_str(),
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
    auto type = AssetUtilities::AssetTypeFromString(search_type_);
    if (ImGui::BeginListBox("## AssetSearchPopup")) {
      for (const auto& metadata : registry | std::views::values) {
        if (type != AssetType::kNone && metadata.type != type) continue;
        std::string preview = metadata.file_path.string();
        if (metadata.type == AssetType::kScript) {
          preview = metadata.file_path.string();
        }
        if (ImGui::Selectable(ui::GenerateLabelID(preview),
                              metadata.handle == pick_)) {
          pick_ = metadata.handle;
        }
      }

      ImGui::EndListBox();
    }
    cancel_button_->Notification(kControlDraw);
    ImGui::SameLine();
    assign_button_->Notification(kControlDraw);
    ImGui::EndPopup();
  }
}

void EditorAssetPicker::AssetSelect() {
  ClosePopup();
  EditedAssetUpdate();
  EmitSignal("AssetSelected", pick_);
}

void EditorAssetPicker::EditedAssetUpdate() const {
	const auto meta = AssetManager::GetMutableMetadata(pick_);
  if (!meta.handle) return;
  edit_button_->SetText(meta.file_path.filename().generic_string());
}

void EditorAssetPicker::Cancel() { ClosePopup(); }

void EditorAssetPicker::ClosePopup() {
  ImGui::CloseCurrentPopup();
  show_popup_ = false;
}
}  // namespace base_engine::editor

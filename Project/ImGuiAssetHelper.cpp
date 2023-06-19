#include "ImGuiAssetHelper.h"

#include <ranges>

#include "AssetManager.h"

namespace base_engine::editor::ImGuiHelper {
bool AssetReferenceField(const std::string& label, AssetHandle* asset_handle,
                         const AssetType asset_type) {
  bool result = false;
  std::string button_text = "Null";
  if (AssetManager::IsAssetHandleValid(*asset_handle)) {
    button_text =
        AssetManager::GetMutableMetadata(*asset_handle).file_path.string();
  }
  const bool clicked = ImGui::Button(button_text.c_str());
  if (ImGui::BeginDragDropTarget()) {
    auto data = ImGui::AcceptDragDropPayload("asset_payload");
    if (data) {
      const auto handle = *static_cast<AssetHandle*>(data->Data);
      *asset_handle = handle;
      result = true;
    	ImGui::EndDragDropTarget();
    }
  }

  ImGui::SameLine();
  ImGui::Text(label.c_str());
  const std::string popup_name = "AssetSearchPopup ## " + label;

  if (clicked) ImGui::OpenPopup(popup_name.c_str());

  const auto& registry = AssetManager::GetAssetRegistry();
  if (ImGui::BeginPopup(popup_name.c_str(),
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
    if (ImGui::BeginListBox(label.c_str())) {
      for (const auto& metadata : registry | std::views::values) {
        if (asset_type != AssetType::kNone && metadata.type != asset_type)
          continue;
        std::string preview = metadata.file_path.string().c_str();
        if (metadata.type == AssetType::kScript) {
          preview = metadata.file_path.string();
        }
        if (ImGui::Selectable(preview.c_str())) {
          *asset_handle = metadata.handle;
          result = true;
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::EndListBox();
    }

    ImGui::EndPopup();
  }

  return result;
}
}  // namespace base_engine::editor::ImGuiHelper

#include "EditorContextMenu.h"

#include "imgui.h"
#include "imgui_internal.h"

namespace base_engine::editor {
EditorContextMenu::EditorContextMenu(const std::string& name) {
  id_++;

  name_ = std::format("{0} ## {1}", name, id_);
}

void EditorContextMenu::Show() {
  ImGui::PushOverrideID(id_);
  ImGui::OpenPopup(name_.c_str());
  ImGui::PopID();
}

void EditorContextMenu::RenderBegin() {
  ImGui::PushOverrideID(id_);

  if (ImGui::BeginPopup(name_.c_str(),
                        ImGuiPopupFlags_MouseButtonRight |
                            ImGuiPopupFlags_NoOpenOverExistingPopup)) {
    for (const auto& [label, func] : this->menu_items_) {
      const auto disabled = std::invoke(func.validate);
      if (disabled) ImGui::BeginDisabled();
      if (ImGui::Selectable(label.c_str())) {
        std::invoke(func.callback);
        ImGui::CloseCurrentPopup();
      }

      if (disabled) ImGui::EndDisabled();
    }
    ImGui::EndPopup();
  }
}

void EditorContextMenu::Render() {}

void EditorContextMenu::RenderEnd() { ImGui::PopID(); }

void EditorContextMenu::BuildRoot() {}

EditorContextMenu::ItemEvents::ItemEvents(const std::function<void()>& callback,
                                          const std::function<bool()>& validate)
    : callback(callback), validate(validate) {}
}  // namespace base_engine::editor

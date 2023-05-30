#include "HierarchyContextMenu.h"

#include <format>

#include "AssetManager.h"
#include "ImGuiAssetHelper.h"
#include "Prefab.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace base_engine::editor {
HierarchyContextMenu::HierarchyContextMenu(const std::string& name) {
  id_++;

  name_ = std::format("{0} ## {1}", name, id_);
}

void HierarchyContextMenu::Show() {
  ImGui::PushOverrideID(id_);
  ImGui::OpenPopup(name_.c_str());
  ImGui::PopID();
}

void HierarchyContextMenu::Render() {
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
  if (ImGui::BeginPopup("Prefab Search",
                        ImGuiPopupFlags_MouseButtonRight |
                            ImGuiPopupFlags_NoOpenOverExistingPopup)) {
    ImGui::Text("Asset: Prefab");
    ImGui::Separator();
    const auto& registry = AssetManager::GetAssetRegistry();

    if (ImGui::BeginListBox("Asset: Prefab")) {
      for (const auto& metadata : registry | std::views::values) {
        if (metadata.type != Prefab::GetStaticType()) continue;
        if (ImGui::Selectable(metadata.file_path.stem().string().c_str())) {
          auto asset = AssetManager::GetAsset<Prefab>(metadata.handle);
          scene_->Instantiate(asset, nullptr, nullptr, nullptr);
          ImGui::CloseCurrentPopup();
        }
      }

      ImGui::EndListBox();
    }

    ImGui::EndPopup();
  }
  ImGui::PopID();
}

void HierarchyContextMenu::BuildRoot() {
  menu_items_.emplace(
      "Remove Object",
      ItemEvents{
          [this] {
            scene_->DestroyEntity(scene_->TryGetEntityWithUUID(target_object_));
          },
          [this] { return !scene_->TryGetEntityWithUUID(target_object_); }});

  menu_items_.emplace("Create Empty", ItemEvents{[this] {
                        const auto entity =
                            scene_->TryGetEntityWithUUID(target_object_);
                        scene_->CreateChildEntity(entity, "Empty");
                      }});
  menu_items_.emplace("Prefab Instance", ItemEvents{[this] {
                        ImGui::PushOverrideID(id_);
                        ImGui::OpenPopup("Prefab Search");
                        ImGui::PopID();
                      }});
  menu_items_.emplace(
      "Prefab Asset", ItemEvents{[this] {
        auto entity = scene_->TryGetEntityWithUUID(target_object_);
        const auto name = entity.GetComponent<component::TagComponent>().tag;
        auto prefab =
            AssetManager::GetEditorAssetManager()->CreateNewAsset<Prefab>(
                name + ".prefab", "");
        prefab->Create(entity);
      }});
}

void HierarchyContextMenu::SetTargetObject(const Ref<Scene>& scene,
                                           const UUID target_object) {
  scene_ = scene;
  target_object_ = target_object;
  name_ = std::format("{0} ## {1}", static_cast<UUID::ValueType>(target_object),
                      id_);
}

HierarchyContextMenu::ItemEvents::ItemEvents(
    const std::function<void()>& callback,
    const std::function<bool()>& validate)
    : callback(callback), validate(validate) {}
}  // namespace base_engine::editor

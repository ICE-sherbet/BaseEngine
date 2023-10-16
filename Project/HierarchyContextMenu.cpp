#include "HierarchyContextMenu.h"

#include <format>

#include "AssetManager.h"
#include "ImGuiAssetHelper.h"
#include "Prefab.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace base_engine::editor {
HierarchyContextMenu::HierarchyContextMenu(const std::string& name)
    : EditorContextMenu(name) {}

void HierarchyContextMenu::Render() {
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
      "Prefab Asset",
      ItemEvents{
          [this] {
            auto entity = scene_->TryGetEntityWithUUID(target_object_);
            const auto name =
                entity.GetComponent<component::TagComponent>().tag;
            auto prefab =
                AssetManager::GetEditorAssetManager()->CreateNewAsset<Prefab>(
                    name + ".prefab", "");
            prefab->Create(entity);
          },
          [this] { return !scene_->TryGetEntityWithUUID(target_object_); }});
}

void HierarchyContextMenu::SetTargetObject(const Ref<Scene>& scene,
                                           const UUID target_object) {
  scene_ = scene;
  target_object_ = target_object;
  name_ = std::format("{0} ## {1}", static_cast<UUID::ValueType>(target_object),
                      id_);
}
}  // namespace base_engine::editor

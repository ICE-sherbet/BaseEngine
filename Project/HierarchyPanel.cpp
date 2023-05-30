#include "HierarchyPanel.h"

#include "SelectManager.h"
#include "imgui.h"

namespace base_engine::editor {
void HierarchyPanel::Initialize(const Ref<Scene>& context) {
  SetSceneContext(context);
  context_menu_ = std::make_unique<HierarchyContextMenu>("Hierarchy");
  context_menu_->BuildRoot();
}

void HierarchyPanel::OnImGuiRender() {
  {
    ImGui::Begin("Hierarchy");

    {
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 3.0f * 3.0f);
      for (const auto entity_view =
               scene_context_->GetAllEntitiesWith<
                   component::IdComponent, component::HierarchyComponent>();
           const auto entity : entity_view) {
        if (entity_view.get<component::HierarchyComponent>(entity)
                .parent_handle == 0) {
          DrawEntityNode({entity, scene_context_.Raw()});
        }
      }

      if (ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
          ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
        context_menu_->SetTargetObject(scene_context_, 0);
        context_menu_->Show();
      }
    }
    context_menu_->Render();
    ImGui::End();
  }
}

void HierarchyPanel::SetSceneContext(const Ref<Scene>& scene) {
  scene_context_ = scene;
}

void HierarchyPanel::DrawEntityNode(const ObjectEntity& entity) {
  const auto& name = entity.GetComponent<component::TagComponent>().tag;
  const auto id = entity.GetComponent<component::IdComponent>().uuid;

  auto tree_flag = ImGuiTreeNodeFlags_OpenOnArrow |
                   ImGuiTreeNodeFlags_OpenOnDoubleClick |
                   ImGuiTreeNodeFlags_SpanAvailWidth;
  if (entity.Children().empty()) tree_flag |= ImGuiTreeNodeFlags_Bullet;
  const bool opened =
      ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(id)),
                        tree_flag, name.c_str());
  const bool left_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
  const bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

  if (left_clicked) {
    SelectManager::Instance()->SelectItem(id);
  }

  if (opened) {
    for (const auto child : entity.Children())
      DrawEntityNode(scene_context_->GetEntityWithUUID(child));
    ImGui::TreePop();
  }
  if (right_clicked) {
    context_menu_->SetTargetObject(scene_context_, entity.GetUUID());
    context_menu_->Show();
  }
}
}  // namespace base_engine::editor

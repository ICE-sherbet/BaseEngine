#include "HierarchyPanel.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "AssetManager.h"
#include "AssetMetadata.h"
#include "CSharpScriptEngine.h"
#include "Prefab.h"
#include "SelectManager.h"
#include "imgui.h"
#include "ImGuiUtilities.h"

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
      ImGui::BeginChild("##Hierarchy Dummy");

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
    context_menu_->RenderBegin();
    context_menu_->Render();
    context_menu_->RenderEnd();
    ImGui::EndChild();

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload =
              ImGui::AcceptDragDropPayload("scene_entity_hierarchy")) {
        const size_t count = payload->DataSize / sizeof(UUID);

        for (size_t i = 0; i < count; i++) {
          const UUID dropped_entity_id =
              *(static_cast<UUID*>(payload->Data) + i);
          const auto dropped_entity =
              scene_context_->GetEntityWithUUID(dropped_entity_id);
          scene_context_->UnParentEntity(dropped_entity, true);
        }
      }
      if (const ImGuiPayload* data =
              ImGui::AcceptDragDropPayload("asset_payload")) {
        const uint64_t count = data->DataSize / sizeof(AssetHandle);

        for (uint64_t i = 0; i < count; i++) {
          const AssetHandle asset_handle =
              *(static_cast<AssetHandle*>(data->Data) + i);
          const AssetMetadata& assetData =
              AssetManager::GetEditorAssetManager()->GetMetadata(asset_handle);
          Ref<Asset> asset = AssetManager::GetAsset<Asset>(asset_handle);
          if (asset->GetAssetType() == AssetType::kPrefab) {
            Ref<Prefab> prefab = asset.As<Prefab>();
            scene_context_->Instantiate(prefab);
          }
        }
      }

      ImGui::EndDragDropTarget();
    }
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
  if (id == selected_entity_) tree_flag |= ImGuiTreeNodeFlags_Selected;
  const bool opened =
      ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(id)),
                        tree_flag, name.c_str());
  const bool left_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
  const bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

  if (left_clicked) {
    selected_entity_ = id;
    SelectManager::Instance()->SelectItem("HierarchyPanel", id);
  }

  if (right_clicked) {
    context_menu_->SetTargetObject(scene_context_, entity.GetUUID());
    context_menu_->Show();
  }

  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    const auto& selected_entities =
        SelectManager::Instance()->GetSelectItem("HierarchyPanel");
    const UUID entity_id = entity.GetUUID();
    auto name = entity.GetComponent<component::TagComponent>().Tag();
    ImGui::TextUnformatted(name.c_str());
    ImGui::SetDragDropPayload("scene_entity_hierarchy", &entity_id,
                              1 * sizeof(UUID));
    ImGui::EndDragDropSource();
  }

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
            "scene_entity_hierarchy",
            ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
      const size_t count = payload->DataSize / sizeof(UUID);

      for (size_t i = 0; i < count; i++) {
        const UUID dropped_entity_id = *(static_cast<UUID*>(payload->Data) + i);
        const auto dropped_entity =
            scene_context_->GetEntityWithUUID(dropped_entity_id);
        scene_context_->SetParentEntity(dropped_entity, entity);
      }
    }
    if (const ImGuiPayload* data = ImGui::AcceptDragDropPayload(
            "asset_payload", ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
      const uint64_t count = data->DataSize / sizeof(AssetHandle);

      for (uint64_t i = 0; i < count; i++) {
        const AssetHandle asset_handle =
            *(static_cast<AssetHandle*>(data->Data) + i);
        const AssetMetadata& assetData =
            AssetManager::GetEditorAssetManager()->GetMetadata(asset_handle);
        Ref<Asset> asset = AssetManager::GetAsset<Asset>(asset_handle);
        if (asset->GetAssetType() == AssetType::kPrefab) {
          Ref<Prefab> prefab = asset.As<Prefab>();
          scene_context_->Instantiate(prefab);
        }
      }
    }

    ImGui::EndDragDropTarget();
  }

  if (opened) {
    for (const auto child : entity.Children())
      DrawEntityNode(scene_context_->GetEntityWithUUID(child));
    ImGui::TreePop();
  }
}
}  // namespace base_engine::editor

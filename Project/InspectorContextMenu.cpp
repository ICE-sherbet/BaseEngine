#include "InspectorContextMenu.h"

#include "imgui.h"

namespace base_engine::editor {

void InspectorContextMenu::Render() {}

void InspectorContextMenu::BuildRoot() {
  menu_items_.emplace(
      "RemoveComponent",
      ItemEvents{[this] {
                   const auto entity =
                       scene_->TryGetEntityWithUUID(target_object_);
                   scene_->GetRegistry().storage(class_id_)->remove(entity);
                   EmitSignal("OnRemovedComponent", class_id_);
                 },
                 [this] {
                   return !scene_->TryGetEntityWithUUID(target_object_) &&
                          class_id_ != 0;
                 }});
}

void InspectorContextMenu::SetTargetObject(const Ref<Scene>& scene,
                                           UUID target_object,
                                           uint32_t class_id) {
  scene_ = scene;
  target_object_ = target_object;
  class_id_ = class_id;
}
}  // namespace base_engine::editor

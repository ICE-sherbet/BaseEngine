// @InspectorContextMenu.h
// @brief
// @author ICE
// @date 2023/07/11
//
// @details

#pragma once
#include "EditorContextMenu.h"
#include "ObjectEntity.h"
#include "Scene.h"

namespace base_engine::editor {
class InspectorContextMenu : public EditorContextMenu {
 public:
  explicit InspectorContextMenu(const std::string& name)
      : EditorContextMenu(name) {}

  void Render() override;
  void BuildRoot() override;
  void SetTargetObject(const Ref<Scene>& scene, UUID target_object,uint32_t class_id);

private:
  Ref<Scene> scene_ = nullptr;
  UUID target_object_ = 0;
  uint32_t class_id_ = 0;
};
}  // namespace base_engine::editor

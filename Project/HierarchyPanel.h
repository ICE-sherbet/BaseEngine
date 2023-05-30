// @HierarchyPanel.h
// @brief Scene内のオブジェクト一覧を表示
// @author ICE
// @date 2023/05/20
//
// @details

#pragma once
#include "EditorPanel.h"
#include "HierarchyContextMenu.h"

namespace base_engine::editor {
class HierarchyPanel : public EditorPanel {
 public:

  void Initialize(const Ref<Scene>& context) override;

  void OnImGuiRender() override;
  void SetSceneContext(const Ref<Scene>& scene) override;

private:

  void DrawEntityNode(const ObjectEntity& entity);
private:
  Ref<Scene> scene_context_;
  std::unique_ptr<HierarchyContextMenu> context_menu_;
};
}  // namespace base_engine::editor

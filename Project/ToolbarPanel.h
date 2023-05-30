// @ToolbarPanel.h
// @brief
// @author ICE
// @date 2023/05/28
//
// @details

#pragma once
#include "EditorPanel.h"
#include "EditorPanelManager.h"

namespace base_engine::editor {
class ToolbarPanel : public EditorPanel {
 public:
  void OnImGuiRender() override;
  void SetSceneContext(const Ref<Scene>& context) override;
  void Initialize(const Ref<Scene>& context) override;
  ToolbarPanel(EditorPanelManager* panel_manager):panel_manager_(panel_manager){};

private:
  void OnScenePlay();
  void OnSceneStop();

  bool is_play_ = false;
  EditorPanelManager* panel_manager_ = nullptr;
  Ref<Scene> context_scene_ = nullptr;
  Ref<Scene> runtime_scene_ = nullptr;
  void* texture_ = nullptr;
};
}  // namespace base_engine::editor

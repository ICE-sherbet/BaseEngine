// @EditorPanelManager.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include <memory>
#include <vector>

#include "Game.h"
#include "Scene.h"

namespace base_engine::editor {
class EditorPanel;

class EditorPanelManager {
 public:
  void Initialize();
  void OnUpdate();
  void OnRender();
  void SetSceneContext(const Ref<Scene>& scene);

  explicit EditorPanelManager(Game* game) : game_(game) {}

  Game* game_ = nullptr;

 private:
  Ref<Scene> scene_context_;
  using StackPanel = std::vector<std::shared_ptr<EditorPanel>>;
  StackPanel panels_;
};
}  // namespace base_engine::editor

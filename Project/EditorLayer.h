// @EditorLayer.h
// @brief
// @author ICE
// @date 2023/05/19
//
// @details

#pragma once
#include <memory>

#include "EditorPanelManager.h"
#include "Game.h"
#include "ILayer.h"

namespace base_engine::editor {
class EditorLayer : public ILayer {
 public:
  explicit EditorLayer(Game* game) : game_(game){};
  void Initialize(const Ref<Scene>& scene) override;
  void OnUpdate() override;
  void OnRender() override;

  void SetSceneContext(const Ref<Scene>& scene) override;

  ~EditorLayer() override;

 private:
  Game* game_ = nullptr;
  Ref<Scene> scene_;
  std::unique_ptr<EditorPanelManager> panel_manager_ = nullptr;
};
}  // namespace base_engine::editor

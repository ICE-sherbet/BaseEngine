// @MenuBarPanel.h
// @brief
// @author ICE
// @date 2023/07/06
//
// @details

#pragma once
#include "EditorPanel.h"

namespace base_engine::editor {
class MenuBarPanel : public EditorPanel {
 public:
  void OnImGuiRender() override;

  void Initialize(const Ref<Scene>& context) override;
  void SetSceneContext(const Ref<Scene>& context) override;

 private:
  Ref<Scene> scene_context_ = nullptr;
};
}  // namespace base_engine::editor

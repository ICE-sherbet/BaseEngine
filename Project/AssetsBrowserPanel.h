// @AssetsBrowserPanel.h
// @brief
// @author ICE
// @date 2023/06/12
//
// @details

#pragma once
#include "EditorPanel.h"

namespace base_engine::editor {
class AssetsBrowserPanel : public EditorPanel {
 public:
  void OnImGuiRender() override;
  void Initialize(const Ref<Scene>& context) override;
  void SetSceneContext(const Ref<Scene>& context) override;

 private:
};
}  // namespace base_engine::editor

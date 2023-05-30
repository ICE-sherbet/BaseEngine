// @EditorPanel.h
// @brief
// @author ICE
// @date 2023/05/20
//
// @details

#pragma once
#include "ConnectableObject.h"
#include "ObjectEntity.h"
#include "Ref.h"
#include "Scene.h"

namespace base_engine::editor {
class EditorPanel : public RefCounted, public ConnectableObject {
 public:
  virtual ~EditorPanel() = default;

  virtual void Initialize(const Ref<Scene>& context) {}

  virtual void OnImGuiRender() = 0;
  virtual void SetSceneContext(const Ref<Scene>& context) {}
};
}  // namespace base_engine::editor

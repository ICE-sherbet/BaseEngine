// @InspectorPanel.h
// @brief
// @author ICE
// @date 2023/05/22
//
// @details

#pragma once
#include "EditorPanel.h"

namespace base_engine::editor {
class InspectorPanel : public EditorPanel {
 public:
  void OnImGuiRender() override;
  void Initialize(const Ref<Scene>& context) override;
  void SetSceneContext(const Ref<Scene>& context) override;
  void PickItem(uint64_t item);

 private:

  void RenderProperties();
  void RenderAddComponentButton(ObjectEntity& object);
  void RenderTag(ObjectEntity& object);
	void RenderTransform(ObjectEntity& object);
	void RenderSprite(ObjectEntity& object);
	void RenderScript(ObjectEntity& object);
	void RenderRigidBody(ObjectEntity& object);
	void RenderCircleShape(ObjectEntity& object);
  Ref<Scene> scene_context_;
  UUID select_item_ = 0;
};
}  // namespace base_engine::editor

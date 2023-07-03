// @InspectorPanel.h
// @brief
// @author ICE
// @date 2023/05/22
//
// @details

#pragma once
#include "EditorPanel.h"
#include "EditorProperty.h"

namespace base_engine::editor {
class InspectorPanel : public EditorPanel {
 public:
  void OnImGuiRender() override;
  void Initialize(const Ref<Scene>& context) override;
  void SetSceneContext(const Ref<Scene>& context) override;
  void PickItem(const std::string& context, uint64_t item);

 private:
  void RenderProperties();
  void RenderAddComponentButton(ObjectEntity& object);
  void RenderProperty(ObjectEntity& object);
  void RenderTag(ObjectEntity& object);
  void RenderTransform(ObjectEntity& object);
  void RenderSprite(ObjectEntity& object);
  void RenderScript(ObjectEntity& object);
  void RenderRigidBody(ObjectEntity& object);
  void RenderCircleShape(ObjectEntity& object);

  void GenerateProperties(ObjectEntity& object);
  void PropertyChanged(const std::string& class_name,
                       const std::string& property, const Variant& value);

  using PropertyPtr = std::shared_ptr<EditorProperty>;
  std::map<std::string, std::vector<PropertyPtr>> properties_map_;
  std::vector<std::string> fixed_class_;
  Ref<Scene> scene_context_;
  UUID select_item_ = 0;
};
}  // namespace base_engine::editor

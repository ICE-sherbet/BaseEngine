// @InspectorPanel.h
// @brief
// @author ICE
// @date 2023/05/22
//
// @details

#pragma once
#include "EditorPanel.h"
#include "EditorProperty.h"
#include "InspectorContextMenu.h"

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

  void GenerateProperties(ObjectEntity& object);
  void PropertyChanged(const std::string& class_name,
                       const std::string& property, const Variant& value);

  void OnRemovedComponent(uint32_t class_id);

  using PropertyPtr = std::shared_ptr<EditorProperty>;
  std::map<std::string, std::vector<PropertyPtr>> properties_map_;
  std::vector<std::string> fixed_class_;
  Ref<Scene> scene_context_;
  UUID select_item_ = 0;
  bool redraw_ = false;
  std::unique_ptr<InspectorContextMenu> context_menu_;
};
}  // namespace base_engine::editor

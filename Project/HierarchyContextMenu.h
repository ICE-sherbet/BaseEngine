// @HierarchyContextMenu.h
// @brief
// @author ICE
// @date 2023/05/26
//
// @details

#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "EditorContextMenu.h"
#include "ObjectEntity.h"
#include "Scene.h"
#include "UUID.h"

namespace base_engine::editor {
class HierarchyContextMenu : public EditorContextMenu {
 public:
  explicit HierarchyContextMenu(const std::string& name);

  void Render() override;

  void BuildRoot() override;

  void SetTargetObject(const Ref<Scene>& scene, UUID target_object);

 private:
  
  Ref<Scene> scene_ = nullptr;
  UUID target_object_ = 0;
};
}  // namespace base_engine::editor

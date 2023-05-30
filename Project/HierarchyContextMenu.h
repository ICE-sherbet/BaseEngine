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

#include "ObjectEntity.h"
#include "Scene.h"
#include "UUID.h"

namespace base_engine::editor {
class HierarchyContextMenu {
 public:
  explicit HierarchyContextMenu(const std::string& name);

  void Show();
  void Render();

  void BuildRoot();

  void SetTargetObject(const Ref<Scene>& scene, UUID target_object);

 private:
  std::string name_;
  struct ItemEvents {
    ItemEvents(
        const std::function<void()>& callback,
        const std::function<bool()>& validate = []() { return false; });

    std::function<void()> callback;
    std::function<bool()> validate;
  };
  std::unordered_map<std::string, ItemEvents> menu_items_;
  inline static size_t id_ = 0;

  Ref<Scene> scene_ = nullptr;
  UUID target_object_ = 0;
};
}  // namespace base_engine::editor

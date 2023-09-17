// @EditorContextMenu.h
// @brief
// @author ICE
// @date 2023/07/11
//
// @details

#pragma once
#include "EditorControl.h"
#include "Ref.h"
namespace base_engine::editor {
class EditorContextMenu : public EditorControl {
 public:
  explicit EditorContextMenu(const std::string& name);

  virtual void Show();

  void RenderBegin();
  virtual void Render();
  void RenderEnd();

  virtual void BuildRoot();

 protected:
  struct ItemEvents {
    ItemEvents(
        const std::function<void()>& callback,
        const std::function<bool()>& validate = []() { return false; });

    std::function<void()> callback;
    std::function<bool()> validate;
  };
  std::unordered_map<std::string, ItemEvents> menu_items_;

  std::string name_;
  inline static size_t id_ = 0;
};
}  // namespace base_engine::editor

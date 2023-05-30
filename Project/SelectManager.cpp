#include "SelectManager.h"
namespace base_engine::editor {
SelectManager::SelectManager() {}

SelectManager* SelectManager::Instance() {
  if (!instance_) {
    instance_ = new SelectManager();
  }
  return instance_;
}

void SelectManager::SelectItem(const UUID item) {
  EmitSignal("SelectItem", static_cast<uint64_t>(item));
  select_item_ = item;
}

UUID SelectManager::GetSelectItem() const { return select_item_; }

SelectManager* SelectManager::instance_ = nullptr;
}  // namespace base_engine::editor

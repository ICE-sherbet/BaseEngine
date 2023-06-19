#include "SelectManager.h"
namespace base_engine::editor {
SelectManager::SelectManager() {}

SelectManager* SelectManager::Instance() {
  if (!instance_) {
    instance_ = new SelectManager();
  }
  return instance_;
}

void SelectManager::SelectItem(const std::string& context, const UUID item) {
  EmitSignal("SelectItem", context, static_cast<uint64_t>(item));
  select_map_[context] = item;
}

UUID SelectManager::GetSelectItem(const std::string& context) const {
  if (!select_map_.contains(context)) return kNullUuid;
  return select_map_.at(context);
}

bool SelectManager::IsSelectItem(const std::string& context, const UUID item) const {
  if (!select_map_.contains(context)) return false;
  return select_map_.at(context) == item;
}

SelectManager* SelectManager::instance_ = nullptr;
}  // namespace base_engine::editor

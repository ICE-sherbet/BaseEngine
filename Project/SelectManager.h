// @SelectManager.h
// @brief
// @author ICE
// @date 2023/05/22
//
// @details

#pragma once
#include "ConnectableObject.h"
#include "UUID.h"

namespace base_engine::editor {
class SelectManager : public ConnectableObject {
  SelectManager();

 public:
  static SelectManager* Instance();

  void SelectItem(const std::string& context, UUID item);

  [[nodiscard]] UUID GetSelectItem(const std::string& context) const;
  [[nodiscard]] bool IsSelectItem(const std::string& context, UUID item) const;

 private:
  using SelectAssetContextMap = std::unordered_map<std::string, UUID>;
  SelectAssetContextMap select_map_;
  static SelectManager* instance_;
};
}  // namespace base_engine::editor

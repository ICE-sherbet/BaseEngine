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

  void SelectItem(UUID item);

  [[nodiscard]] UUID GetSelectItem() const;

private:
  UUID select_item_;
  static SelectManager* instance_;
};
}  // namespace base_engine::editor

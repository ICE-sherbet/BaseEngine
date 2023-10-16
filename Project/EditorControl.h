// @EditorControl.h
// @brief
// @author ICE
// @date 2023/06/27
//
// @details

#pragma once
#include "ConnectableObject.h"

namespace base_engine::editor {
class EditorControl : public ConnectableObject {
 protected:
  enum {
    kControlDraw = 100,
  };

 public:
  virtual void Notification(int type) {}
};
}  // namespace base_engine::editor

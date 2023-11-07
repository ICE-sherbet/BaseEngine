// @ImGuiLayer.h
// @brief
// @author ICE
// @date 2023/11/07
//
// @details

#pragma once

namespace base_engine {
class ImGuiLayer {
 public:
  virtual void Init() = 0;

  virtual void Begin() = 0;
  virtual void End() = 0;
};
}  // namespace base_engine

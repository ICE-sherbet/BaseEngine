// @IBaseEngineWindow.h
// @brief
// @author ICE
// @date 2023/10/17
//
// @details

#pragma once

namespace base_engine {
class IBaseEngineWindow {
 public:
  virtual ~IBaseEngineWindow() = default;
  static IBaseEngineWindow* Create();
  virtual void Init() = 0;
  virtual void Update() = 0;
};
}  // namespace base_engine
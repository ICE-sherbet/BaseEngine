// @IBecsSystem.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
namespace base_engine {
class ISystem {
 public:
  virtual void OnInit() = 0;

  virtual void OnUpdate() = 0;
};
}  // namespace base_engine
// @MonoGlue.h
// @brief
// @author ICE
// @date 2023/04/13
//
// @details

#pragma once
#include "ScriptTypes.h"

namespace base_engine {

class MonoGlue {
 public:
  explicit MonoGlue();
  void RegisterGlue() const;
  ~MonoGlue();

 private:
  class Impl;
  std::unique_ptr<MonoGlue::Impl> impl_;
};

}  // namespace base_engine

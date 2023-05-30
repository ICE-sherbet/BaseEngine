// @Connection.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include <map>

#include "Callable.h"

namespace base_engine {
struct Connection {
  Callable callable;
};
struct SignalData {
  struct Slot {
    Connection conn;
  };

  std::map<Callable, Slot> slot_map;
};
}  // namespace base_engine

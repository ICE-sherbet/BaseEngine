// @ConnectableObject.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include <span>
#include <string>
#include <unordered_map>

#include "Connection.h"
#include "ScriptTypes.h"

namespace base_engine {
class ConnectableObject {
 public:
  bool Connect(const std::string& signal, const Callable& callable) {
    if (!signal_map_.contains(signal)) {
      signal_map_[signal] = SignalData();
    }
    SignalData* sd = &signal_map_[signal];
    ;
    const Callable target = callable;

    SignalData::Slot slot{};
    {
      Connection conn;
      conn.callable = target;
      slot.conn = conn;
    }

    sd->slot_map[target] = slot;

    return true;
  }

  template <typename... VarArgs>
  bool EmitSignal(const std::string& signal, VarArgs... args) {
    Variant variant_args[sizeof...(args) + 1] = {static_cast<Variant>(args)..., Variant()};
    const Variant* argptrs[sizeof...(args) + 1];
    for (uint32_t i = 0; i < sizeof...(args); i++) {
      argptrs[i] = &variant_args[i];
    }
    return EmitSignalImpl(
        signal, sizeof...(args) == 0 ? nullptr : (const Variant**)argptrs,
        sizeof...(args));
  }

 private:
  bool EmitSignalImpl(const std::string& signal, const Variant** args,
                      const size_t args_count) {
    if (!signal_map_.contains(signal)) {
      return false;
    }
    const SignalData* s = &signal_map_[signal];

    for (auto slot_map = s->slot_map; auto& map : slot_map) {
      Connection& c = map.second.conn;

      c.callable.Call(args, args_count);
    }
  }

  std::unordered_map<std::string, SignalData> signal_map_;
  std::vector<Connection> connections_;
};
}  // namespace base_engine

// @Callable.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include <cstdint>
#include <string>

#include "BinderCommon.h"
#include "ScriptTypes.h"

namespace base_engine {
class CallableSignalBase {
 public:
  virtual void Call(const Variant** arguments, int arg_count) const = 0;
};
template <class T, class... P>
class CallableSignal : public CallableSignalBase {
  struct Data {
    T* instance;
    void (T::*method)(P...);
  } data_;

 public:
  explicit CallableSignal(T* instance, void (T::*method)(P...))
      : data_(instance, method) {}

  void Call(const Variant** arguments, int arg_count) const override {
    call_with_variant_args(data_.instance, data_.method, arguments, arg_count);
  }
};
class Callable {
 protected:
  std::shared_ptr<CallableSignalBase> object_ = nullptr;

 public:
  Callable() = default;

  explicit Callable(std::shared_ptr<CallableSignalBase> object)
      : object_(std::move(object)) {}

  void Call(const Variant** arguments, int arg_count) const;

  friend bool operator<(const Callable& lhs, const Callable& rhs) {
    return lhs.object_ < rhs.object_;
  }

  friend bool operator<=(const Callable& lhs, const Callable& rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const Callable& lhs, const Callable& rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const Callable& lhs, const Callable& rhs) {
    return !(lhs < rhs);
  }
};

template <class T, class... P>
Callable make_callable_function_pointer(T* instance, void (T::*method)(P...)) {
  return Callable{std::make_shared<CallableSignal<T, P...>>(instance, method)};
}
}  // namespace base_engine

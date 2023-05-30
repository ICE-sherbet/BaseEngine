// @MethodBind.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include "ScriptTypes.h"
#include "BinderCommon.h"

namespace base_engine {

class MethodBind {
 public:
  virtual Variant call(void *p_object, const Variant **p_args,
                       int p_arg_count) const = 0;
};
template <class T, class... P>
class MethodBindT : public MethodBind {
  void (T::*method_)(P...) = nullptr;

 public:
  Variant call(void *p_object, const Variant **p_args,
               int p_arg_count) const override {
    call_with_variant_args_dv(static_cast<T*>(p_object), method_, p_args,
                              p_arg_count);
    return Variant{};
  }

  MethodBindT(void (T::*method)(P...)) : method_(method) {}
};
}  // namespace base_engine

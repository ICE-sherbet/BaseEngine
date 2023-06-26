// @MethodBind.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include "BinderCommon.h"
#include "ScriptTypes.h"

namespace base_engine {

class MethodBind {
  std::string class_name_;
  std::string method_name_;

 public:
  virtual Variant call(void *p_object, const Variant **p_args,
                       int p_arg_count) const = 0;

  [[nodiscard]] std::string ClassName() const { return class_name_; }

  void SetClassName(const std::string& class_ame) { class_name_ = class_ame; }

  [[nodiscard]] std::string MethodName() const { return method_name_; }

  void SetMethodName(const std::string& method_name) {
    method_name_ = method_name;
  }
};
template <class T, class... P>
class MethodBindT : public MethodBind {
  void (T::*method_)(P...) = nullptr;

 public:
  Variant call(void *p_object, const Variant **p_args,
               int p_arg_count) const override {
    call_with_variant_args_dv(static_cast<T *>(p_object), method_, p_args,
                              p_arg_count);
    return Variant{};
  }

  MethodBindT(void (T::*method)(P...)) : method_(method) {}
};

template <class T, class... P>
std::shared_ptr<MethodBind> CreateMethodBind(void (T::*p_method)(P...)) {
  auto result = (std::make_shared<MethodBindT<T, P...>>)(p_method);
  result->SetClassName(T::_GetClassNameStatic());
  return result;
}
template <class T, class... P>
class MethodBindTC : public MethodBind {
  void (T::*method_)(P...) const = nullptr;

 public:
  Variant call(void *p_object, const Variant **p_args,
               int p_arg_count) const override {
    call_with_variant_args_const_dv(static_cast<T *>(p_object), method_, p_args,
                              p_arg_count);
    return Variant{};
  }

  MethodBindTC(void (T::*method)(P...) const) : method_(method) {}
};
template <class T, class... P>
std::shared_ptr<MethodBind> CreateMethodBind(void (T::*p_method)(P...) const) {
  auto result = (std::make_shared<MethodBindTC<T, P...>>)(p_method);
  result->SetClassName(T::_GetClassNameStatic());
  return result;
}
template <class T, class R, class... P>
class MethodBindTR : public MethodBind {
  R (T::*method_)(P...) = nullptr;

 public:
  Variant call(void *p_object, const Variant **p_args,
               int p_arg_count) const override {
    Variant result;
    call_with_variant_args_ret_dv(static_cast<T *>(p_object), method_, p_args,
                                  p_arg_count, result);
    return result;
  }

  MethodBindTR(R (T::*method)(P...)) : method_(method) {}
};

template <class T, class R, class... P>
std::shared_ptr<MethodBind> CreateMethodBind(R (T::*p_method)(P...)) {
  auto result = (std::make_shared<MethodBindTR<T, R, P...>>)(p_method);
  result->SetClassName(T::_GetClassNameStatic());
  return result;
}

template <class T, class R, class... P>
class MethodBindTRC : public MethodBind {
  R (T::*method_)(P...) const = nullptr;

 public:
  Variant call(void *p_object, const Variant **p_args,
               int p_arg_count) const override {
    Variant result;
    call_with_variant_args_ret_const_dv(static_cast<T *>(p_object), method_, p_args,
                                  p_arg_count, result);
    return result;
  }

  MethodBindTRC(R (T::*method)(P...) const) : method_(method) {}
};
template <class T, class R, class... P>
std::shared_ptr<MethodBind> CreateMethodBind(R (T::*p_method)(P...) const) {
  auto result = (std::make_shared<MethodBindTRC<T, R, P...>>)(p_method);
  result->SetClassName(T::_GetClassNameStatic());
  return result;
}
}  // namespace base_engine

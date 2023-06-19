// @BinderCommon.h
// @brief
// @author ICE
// @date 2023/05/21
//
// @details

#pragma once
#include "ScriptTypes.h"

namespace base_engine {

template <size_t... Is>
struct IndexSequence {};

template <size_t N, size_t... Is>
struct BuildIndexSequence : BuildIndexSequence<N - 1, N - 1, Is...> {};

template <size_t... Is>
struct BuildIndexSequence<0, Is...> : IndexSequence<Is...> {};

template <class T>
struct VariantCaster {
  static T cast(const Variant &p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    return static_cast<TStripped>(p_variant);
  }
};
template <class T>
struct VariantCaster<T &> {
  static T cast(const Variant &p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    return static_cast<TStripped>(p_variant);
  }
};
template <class T>
struct VariantCaster<const T &> {
  static T cast(const Variant &p_variant) {
    using TStripped = std::remove_pointer_t<T>;
    return static_cast<TStripped>(p_variant);
  }
};
template <class T, class... P, size_t... Is>
void call_with_variant_args_helper(T *p_instance, void (T::*p_method)(P...),
                                   const Variant **p_args,
                                   IndexSequence<Is...>) {
  (p_instance->*p_method)(VariantCaster<P>::cast(*p_args[Is])...);
}

template <class T, class... P>
void call_with_variant_args_dv(T *p_instance, void (T::*p_method)(P...),
                               const Variant **p_args, int p_argcount) {
  int32_t missing = (int32_t)sizeof...(P) - (int32_t)p_argcount;

  const Variant
      *args[sizeof...(P) == 0 ? 1 : sizeof...(P)];  // avoid zero sized array
  for (int32_t i = 0; i < (int32_t)sizeof...(P); i++) {
    if (i < p_argcount) {
      args[i] = p_args[i];
    }
  }

  call_with_variant_args_helper(p_instance, p_method, args,
                                BuildIndexSequence<sizeof...(P)>{});
}

template <class T, class... P>
void call_with_variant_args(T *p_instance, void (T::*p_method)(P...),
                            const Variant **p_args, int p_argcount) {
  call_with_variant_args_helper<T, P...>(p_instance, p_method, p_args,
                                         BuildIndexSequence<sizeof...(P)>{});
}
}  // namespace base_engine

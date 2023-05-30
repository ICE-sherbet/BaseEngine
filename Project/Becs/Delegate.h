// @Delegate.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "TypeTraits.h"

namespace becs {

/*! @brief 曖昧さ回避のための引数型 */
template <auto>
struct connect_arg_t {
  explicit connect_arg_t() = default;
};
template <auto Candidate>
inline constexpr connect_arg_t<Candidate> connect_arg{};

namespace internal {

template <typename Ret, typename... Args>
constexpr auto function_pointer(Ret (*)(Args...)) -> Ret (*)(Args...);

template <typename Ret, typename Type, typename... Args, typename Other>
constexpr auto function_pointer(Ret (*)(Type, Args...), Other &&)
    -> Ret (*)(Args...);

template <typename Class, typename Ret, typename... Args, typename... Other>
constexpr auto function_pointer(Ret (Class::*)(Args...), Other &&...)
    -> Ret (*)(Args...);

template <typename Class, typename Ret, typename... Args, typename... Other>
constexpr auto function_pointer(Ret (Class::*)(Args...) const, Other &&...)
    -> Ret (*)(Args...);

template <typename Class, typename Type, typename... Other>
constexpr auto function_pointer(Type Class::*, Other &&...) -> Type (*)();

template <typename... Type>
using function_pointer_t = decltype(function_pointer(std::declval<Type>()...));

template <typename... Class, typename Ret, typename... Args>
[[nodiscard]] constexpr auto index_sequence_for(Ret (*)(Args...)) {
  return std::index_sequence_for<Class..., Args...>{};
}

}  // namespace internal

template <typename>
class delegate;

/**
 * \brief メンバー関数を送信するためのクラス
 * インスタンスを削除する前にインスタンスを切る必要がある
 * \tparam Ret 
 * \tparam Args 
 */
template <typename Ret, typename... Args>
class delegate<Ret(Args...)> {
  template <auto Candidate, std::size_t... Index>
  [[nodiscard]] auto wrap(std::index_sequence<Index...>) noexcept {
    return [](const void *, Args... args) -> Ret {
      [[maybe_unused]] const auto arguments =
          std::forward_as_tuple(std::forward<Args>(args)...);
      return static_cast<Ret>(std::invoke(
          Candidate,
          std::forward<type_list_element_t<Index, type_list<Args...>>>(
              std::get<Index>(arguments))...));
    };
  }

  template <auto Candidate, typename Type, std::size_t... Index>
  [[nodiscard]] auto wrap(Type &, std::index_sequence<Index...>) noexcept {
    return [](const void *payload, Args... args) -> Ret {
      [[maybe_unused]] const auto arguments =
          std::forward_as_tuple(std::forward<Args>(args)...);
      Type *curr = static_cast<Type *>(
          const_cast<constness_as_t<void, Type> *>(payload));
      return static_cast<Ret>(std::invoke(
          Candidate, *curr,
          std::forward<type_list_element_t<Index, type_list<Args...>>>(
              std::get<Index>(arguments))...));
    };
  }

  template <auto Candidate, typename Type, std::size_t... Index>
  [[nodiscard]] auto wrap(Type *, std::index_sequence<Index...>) noexcept {
    return [](const void *payload, Args... args) -> Ret {
      [[maybe_unused]] const auto arguments =
          std::forward_as_tuple(std::forward<Args>(args)...);
      Type *curr = static_cast<Type *>(
          const_cast<constness_as_t<void, Type> *>(payload));
      return static_cast<Ret>(std::invoke(
          Candidate, curr,
          std::forward<type_list_element_t<Index, type_list<Args...>>>(
              std::get<Index>(arguments))...));
    };
  }

 public:
  using function_type = Ret(const void *, Args...);
  using type = Ret(Args...);
  using result_type = Ret;

  delegate() noexcept : instance{nullptr}, fn{nullptr} {}

  template <auto Candidate, typename... Type>
  delegate(connect_arg_t<Candidate>, Type &&...value_or_instance) noexcept {
    connect<Candidate>(std::forward<Type>(value_or_instance)...);
  }

  delegate(function_type *function, const void *payload = nullptr) noexcept {
    connect(function, payload);
  }

  template <auto Candidate>
  void connect() noexcept {
    instance = nullptr;

    if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
      fn = [](const void *, Args... args) -> Ret {
        return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
      };
    } else if constexpr (std::is_member_pointer_v<decltype(Candidate)>) {
      fn = wrap<Candidate>(internal::index_sequence_for<
                           type_list_element_t<0, type_list<Args...>>>(
          internal::function_pointer_t<decltype(Candidate)>{}));
    } else {
      fn = wrap<Candidate>(internal::index_sequence_for(
          internal::function_pointer_t<decltype(Candidate)>{}));
    }
  }

  template <auto Candidate, typename Type>
  void connect(Type &value_or_instance) noexcept {
    instance = &value_or_instance;

    if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Type &,
                                        Args...>) {
      fn = [](const void *payload, Args... args) -> Ret {
        Type *curr = static_cast<Type *>(
            const_cast<constness_as_t<void, Type> *>(payload));
        return Ret(std::invoke(Candidate, *curr, std::forward<Args>(args)...));
      };
    } else {
      fn = wrap<Candidate>(
          value_or_instance,
          internal::index_sequence_for(
              internal::function_pointer_t<decltype(Candidate), Type>{}));
    }
  }

  template <auto Candidate, typename Type>
  void connect(Type *value_or_instance) noexcept {
    instance = value_or_instance;

    if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Type *,
                                        Args...>) {
      fn = [](const void *payload, Args... args) -> Ret {
        Type *curr = static_cast<Type *>(
            const_cast<constness_as_t<void, Type> *>(payload));
        return Ret(std::invoke(Candidate, curr, std::forward<Args>(args)...));
      };
    } else {
      fn = wrap<Candidate>(
          value_or_instance,
          internal::index_sequence_for(
              internal::function_pointer_t<decltype(Candidate), Type>{}));
    }
  }

  /**
   * @brief Connects an user defined function with optional payload to a
   * delegate.
   *
   * The delegate isn't responsible for the connected object or the payload.
   * Users must always guarantee that the lifetime of an instance overcomes
   * the one of the delegate.<br/>
   * The payload is returned as the first argument to the target function in
   * all cases.
   *
   * @param function Function to connect to the delegate.
   * @param payload User defined arbitrary data.
   */
  void connect(function_type *function,
               const void *payload = nullptr) noexcept {
    instance = payload;
    fn = function;
  }

  void reset() noexcept {
    instance = nullptr;
    fn = nullptr;
  }


  [[nodiscard]] function_type *target() const noexcept { return fn; }

  [[nodiscard]] const void *data() const noexcept { return instance; }

  Ret operator()(Args... args) const {
    return fn(instance, std::forward<Args>(args)...);
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return !(fn == nullptr);
  }

  [[nodiscard]] bool operator==(
      const delegate<Ret(Args...)> &other) const noexcept {
    return fn == other.fn && instance == other.instance;
  }

 private:
  const void *instance;
  function_type *fn;
};

template <typename Ret, typename... Args>
[[nodiscard]] bool operator!=(const delegate<Ret(Args...)> &lhs,
                              const delegate<Ret(Args...)> &rhs) noexcept {
  return !(lhs == rhs);
}

template <auto Candidate>
delegate(connect_arg_t<Candidate>) -> delegate<
    std::remove_pointer_t<internal::function_pointer_t<decltype(Candidate)>>>;

template <auto Candidate, typename Type>
delegate(connect_arg_t<Candidate>, Type &&) -> delegate<std::remove_pointer_t<
    internal::function_pointer_t<decltype(Candidate), Type>>>;

template <typename Ret, typename... Args>
delegate(Ret (*)(const void *, Args...), const void * = nullptr)
    -> delegate<Ret(Args...)>;

}  // namespace becs
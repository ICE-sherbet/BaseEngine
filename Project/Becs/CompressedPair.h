// @CompressedPair.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "TypeTraits.h"

namespace becs {
template <typename First, typename Second>
class compressed_pair {
 public:
  using first_type = First;
  using second_type = Second;

  template <bool Dummy = true>
    requires std::is_default_constructible_v<First> &&
                 std::is_default_constructible_v<Second>
  constexpr compressed_pair() noexcept(
      std::is_nothrow_default_constructible_v<First>
          &&std::is_nothrow_default_constructible_v<Second>)
      : first_base_{}, second_base_{} {}

  constexpr compressed_pair(const compressed_pair &other) noexcept(
      std::is_nothrow_copy_constructible_v<First>
          &&std::is_nothrow_copy_constructible_v<Second>) = default;

  constexpr compressed_pair(compressed_pair &&other) noexcept(
      std::is_nothrow_move_constructible_v<First>
          &&std::is_nothrow_move_constructible_v<Second>) = default;

  template <typename Arg, typename Other>
  constexpr compressed_pair(Arg &&arg, Other &&other) noexcept(
      std::is_nothrow_constructible_v<First, Arg>
          &&std::is_nothrow_constructible_v<Second, Other>)
      : first_base_{std::forward<Arg>(arg)},
        second_base_{std::forward<Other>(other)} {}

  template <typename... Args, typename... Other>
  constexpr compressed_pair(
      std::piecewise_construct_t, std::tuple<Args...> args,
      std::tuple<Other...>
          other) noexcept(std::is_nothrow_constructible_v<First, Args...> &&
                              std::is_nothrow_constructible_v<Second, Other...>)
      : first_base_{std::move(args), std::index_sequence_for<Args...>{}},
        second_base_{std::move(other), std::index_sequence_for<Other...>{}} {}

  constexpr compressed_pair &operator=(const compressed_pair &other) noexcept(
      std::is_nothrow_copy_assignable_v<First>
          &&std::is_nothrow_copy_assignable_v<Second>) = default;

  constexpr compressed_pair &operator=(compressed_pair &&other) noexcept(
      std::is_nothrow_move_assignable_v<First>
          &&std::is_nothrow_move_assignable_v<Second>) = default;

  [[nodiscard]] constexpr first_type &first() noexcept { return first_base_; }

  [[nodiscard]] constexpr const first_type &first() const noexcept {
    return first_base_;
  }

  [[nodiscard]] constexpr second_type &second() noexcept {
    return second_base_;
  }

  [[nodiscard]] constexpr const second_type &second() const noexcept {
    return second_base_;
  }

  constexpr void swap(compressed_pair &other) noexcept(
      std::is_nothrow_swappable_v<first_type>
          &&std::is_nothrow_swappable_v<second_type>) {
    using std::swap;
    swap(first(), other.first());
    swap(second(), other.second());
  }

  template <std::size_t Index>
  constexpr decltype(auto) get() noexcept {
    if constexpr (Index == 0u) {
      return first();
    } else {
      static_assert(Index == 1u, "Index out of bounds");
      return second();
    }
  }

  template <std::size_t Index>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    if constexpr (Index == 0u) {
      return first();
    } else {
      static_assert(Index == 1u, "Index out of bounds");
      return second();
    }
  }

 private:
  [[no_unique_address]] First first_base_;
  [[no_unique_address]] Second second_base_;
};

template <typename Type, typename Other>
compressed_pair(Type &&, Other &&)
    -> compressed_pair<std::decay_t<Type>, std::decay_t<Other>>;

template <typename First, typename Second>
inline constexpr void swap(compressed_pair<First, Second> &lhs,
                           compressed_pair<First, Second> &rhs) {
  lhs.swap(rhs);
}
};  // namespace becs

namespace std {

template <typename First, typename Second>
struct tuple_size<becs::compressed_pair<First, Second>>
    : integral_constant<size_t, 2u> {};

template <size_t Index, typename First, typename Second>
struct tuple_element<Index, becs::compressed_pair<First, Second>>
    : conditional<Index == 0u, First, Second> {
  static_assert(Index < 2u, "Index out of bounds");
};
}  // namespace std

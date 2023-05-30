// @BecsComponent.h
// @brief
// @author ICE
// @date 2023/03/18
//
// @details

#pragma once
#include <cstddef>
#include <type_traits>

namespace becs {

namespace internal {

template <typename Type, typename = void>
struct in_place_delete
    : std::bool_constant<!(std::is_move_constructible_v<Type> &&
                           std::is_move_assignable_v<Type>)> {};

template <>
struct in_place_delete<void> : std::false_type {};

template <typename Type>
struct in_place_delete<Type, std::enable_if_t<Type::in_place_delete>>
    : std::true_type {};

template <typename Type, typename = void>
struct page_size : std::integral_constant<std::size_t, !std::is_empty_v<Type> *
                                                           kBePackedPage> {};

template <>
struct page_size<void> : std::integral_constant<std::size_t, 0u> {};

template <typename Type>
struct page_size<Type, std::enable_if_t<std::is_convertible_v<
                           decltype(Type::page_size), std::size_t>>>
    : std::integral_constant<std::size_t, Type::page_size> {};

}  // namespace internal

/**
 * \brief コンポーネントであるということを識別するためのタグ
 * \tparam Type コンポーネント型
 */
template <typename Type, typename = void>
struct component_traits {
  static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Unsupported type");

  using type = Type;

  /*! @brief Pointer stability, default is `false`. */
  static constexpr bool in_place_delete =
      internal::in_place_delete<Type>::value;

  static constexpr std::size_t page_size = internal::page_size<Type>::value;
};

}  // namespace becs

// @HashedString.h
// @brief
// @author ICE
// @date 2023/03/16
//
// @details

#pragma once

#include <cstddef>
#include <cstdint>

#include "config.h"

namespace becs {

namespace internal {

template <typename>
struct fnv1a_traits;

template <>
struct fnv1a_traits<std::uint32_t> {
  using type = std::uint32_t;
  static constexpr std::uint32_t offset = 2166136261;
  static constexpr std::uint32_t prime = 16777619;
};

template <>
struct fnv1a_traits<std::uint64_t> {
  using type = std::uint64_t;
  static constexpr std::uint64_t offset = 14695981039346656037ull;
  static constexpr std::uint64_t prime = 1099511628211ull;
};

template <typename Char>
struct basic_hashed_string {
  using value_type = Char;
  using size_type = std::size_t;
  using hash_type = id_type;

  const value_type *repr;
  size_type length;
  hash_type hash;
};

}  // namespace internal
template <typename Char>
class basic_hashed_string : internal::basic_hashed_string<Char> {
  using base_type = internal::basic_hashed_string<Char>;
  using traits_type = internal::fnv1a_traits<id_type>;

  struct const_wrapper {
    constexpr const_wrapper(const Char *str) noexcept : repr{str} {}

    const Char *repr;
  };

  [[nodiscard]] static constexpr auto helper(const Char *str) noexcept {
    base_type base{str, 0u, traits_type::offset};

    for (; str[base.length]; ++base.length) {
      base.hash =
          (base.hash ^ static_cast<traits_type::type>(str[base.length])) *
          traits_type::prime;
    }

    return base;
  }

  [[nodiscard]] static constexpr auto helper(const Char *str,
                                             const std::size_t len) noexcept {
    base_type base{str, len, traits_type::offset};

    for (size_type pos{}; pos < len; ++pos) {
      base.hash = (base.hash ^ static_cast<traits_type::type>(str[pos])) *
                  traits_type::prime;
    }

    return base;
  }

 public:
  using value_type = typename base_type::value_type;
  using size_type = typename base_type::size_type;
  using hash_type = typename base_type::hash_type;

  [[nodiscard]] static constexpr hash_type value(const value_type *str,
                                                 const size_type len) noexcept {
    return basic_hashed_string{str, len};
  }

  template <std::size_t N>
  [[nodiscard]] static constexpr hash_type value(
      const value_type (&str)[N]) noexcept {
    return basic_hashed_string{str};
  }

  [[nodiscard]] static constexpr hash_type value(
      const_wrapper wrapper) noexcept {
    return basic_hashed_string{wrapper};
  }

  constexpr basic_hashed_string() noexcept : base_type{} {}

  constexpr basic_hashed_string(const value_type *str,
                                const size_type len) noexcept
      : base_type{helper(str, len)} {}

  template <std::size_t N>
  constexpr basic_hashed_string(const value_type (&str)[N]) noexcept
      : base_type{helper(str)} {}

  explicit constexpr basic_hashed_string(const_wrapper wrapper) noexcept
      : base_type{helper(wrapper.repr)} {}

  [[nodiscard]] constexpr size_type size() const noexcept {
    return base_type::length;
  }

  [[nodiscard]] constexpr const value_type *data() const noexcept {
    return base_type::repr;
  }

  [[nodiscard]] constexpr hash_type value() const noexcept {
    return base_type::hash;
  }

  [[nodiscard]] constexpr operator const value_type *() const noexcept {
    return data();
  }

  [[nodiscard]] constexpr operator hash_type() const noexcept {
    return value();
  }
};

template <typename Char>
basic_hashed_string(const Char *str, const std::size_t len)
    -> basic_hashed_string<Char>;

template <typename Char, std::size_t N>
basic_hashed_string(const Char (&str)[N]) -> basic_hashed_string<Char>;

template <typename Char>
[[nodiscard]] constexpr bool operator==(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return lhs.value() == rhs.value();
}

template <typename Char>
[[nodiscard]] constexpr bool operator!=(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename Char>
[[nodiscard]] constexpr bool operator<(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return lhs.value() < rhs.value();
}

template <typename Char>
[[nodiscard]] constexpr bool operator<=(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return !(rhs < lhs);
}

template <typename Char>
[[nodiscard]] constexpr bool operator>(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return rhs < lhs;
}

template <typename Char>
[[nodiscard]] constexpr bool operator>=(
    const basic_hashed_string<Char> &lhs,
    const basic_hashed_string<Char> &rhs) noexcept {
  return !(lhs < rhs);
}

using hashed_string = basic_hashed_string<char>;

using hashed_wstring = basic_hashed_string<wchar_t>;

inline namespace literals {

[[nodiscard]] constexpr hashed_string operator"" _hs(const char *str,
                                                     std::size_t) noexcept {
  return hashed_string{str};
}

[[nodiscard]] constexpr hashed_wstring operator"" _hws(const wchar_t *str,
                                                       std::size_t) noexcept {
  return hashed_wstring{str};
}

}  // namespace literals

}  // namespace becs
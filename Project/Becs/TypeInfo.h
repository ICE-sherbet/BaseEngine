// @TypeInfo.h
// @brief
// @author ICE
// @date 2023/03/16
//
// @details

#pragma once
#include <atomic>
#include <string_view>
#include <type_traits>
#include <utility>

#include "HashedString.h"
#include "config.h"

namespace becs {

namespace internal {

struct type_index final {
  [[nodiscard]] static id_type next() noexcept {
    static std::atomic<id_type> value{};
    return value++;
  }
};

template <typename Type>
[[nodiscard]] constexpr auto stripped_type_name() noexcept {
#if defined BECS_PRETTY_FUNCTION
  std::string_view pretty_function{BECS_PRETTY_FUNCTION};
  auto first = pretty_function.find_first_not_of(
      ' ', pretty_function.find_first_of(BECS_PRETTY_FUNCTION_PREFIX) + 1);
  auto value = pretty_function.substr(
      first, pretty_function.find_last_of(BECS_PRETTY_FUNCTION_SUFFIX) - first);
  return value;
#else
  return std::string_view{""};
#endif
}

template <typename Type, auto = stripped_type_name<Type>().find_first_of('.')>
[[nodiscard]] static constexpr std::string_view type_name(int) noexcept {
  constexpr auto value = stripped_type_name<Type>();
  return value;
}

template <typename Type>
[[nodiscard]] static std::string_view type_name(char) noexcept {
  static const auto value = stripped_type_name<Type>();
  return value;
}

template <typename Type, auto = stripped_type_name<Type>().find_first_of('.')>
[[nodiscard]] static constexpr id_type type_hash(int) noexcept {
  constexpr auto stripped = stripped_type_name<Type>();
  constexpr auto value = hashed_string::value(stripped.data(), stripped.size());
  return value;
}
[[nodiscard]] static constexpr id_type type_hash(int) noexcept {
  constexpr auto stripped = stripped_type_name<int>();
  constexpr auto value = hashed_string::value(stripped.data(), stripped.size());
  return value;
}
template <typename Type>
[[nodiscard]] static id_type type_hash(char) noexcept {
  static const auto value = [](const auto stripped) {
    return hashed_string::value(stripped.data(), stripped.size());
  }(stripped_type_name<Type>());
  return value;
}

}  // namespace internal

/**
 * \brief 型のシーケンシャルな識別子
 * @tparam Type シーケンシャルな識別子を生成する型
 */
template <typename Type, typename = void>
struct type_index final {
  /**
   * \brief シーケンシャルな識別子を返す
   * @return 型のシーケンシャルな識別子
   */
  [[nodiscard]] static id_type value() noexcept {
    static const id_type value = internal::type_index::next();
    return value;
  }

  /*! @copydoc value */
  [[nodiscard]] constexpr operator id_type() const noexcept { return value(); }
};

/**
 * \brief 型のハッシュ
 * @tparam Type ハッシュ化する型
 */
template <typename Type, typename = void>
struct type_hash final {
  /**
   * \brief 型をハッシュ化した値を返す
   * @return 型のハッシュを返す
   */
#if defined BECS_PRETTY_FUNCTION
  [[nodiscard]] static constexpr id_type value() noexcept {
    return internal::type_hash<Type>(0);
#else
  [[nodiscard]] static constexpr id_type value() noexcept {
    return type_index<Type>::value();
#endif
  }

  /*! @copydoc value */
  [[nodiscard]] constexpr operator id_type() const noexcept { return value(); }
};

/**
 * \brief RTTIではなく、コンパイル時定数として型名を扱う
 * @tparam Type 名前を生成する型
 */
template <typename Type, typename = void>
struct type_name final {
  /**
   * \brief 型の名前を返す
   * @return 型の名前
   */
  [[nodiscard]] static constexpr std::string_view value() noexcept {
    return internal::type_name<Type>(0);
  }

  /*! @copydoc value */
  [[nodiscard]] constexpr operator std::string_view() const noexcept {
    return value();
  }
};

/*
 * \brief 型に関する実装固有の情報
 */
struct type_info final {
  /**
   * \brief 指定された型の型情報オブジェクトを構築します
   * @tparam Type 型情報オブジェクトを構築する型
   */
  template <typename Type>
  constexpr type_info(std::in_place_type_t<Type>) noexcept
      : seq{type_index<
            std::remove_cv_t<std::remove_reference_t<Type>>>::value()},
        identifier{type_hash<
            std::remove_cv_t<std::remove_reference_t<Type>>>::value()},
        alias{type_name<
            std::remove_cv_t<std::remove_reference_t<Type>>>::value()} {}

  /**
   * \brief Type index.
   * @return Type index.
   */
  [[nodiscard]] constexpr id_type index() const noexcept { return seq; }

  /**
   * \brief Type hash.
   * @return Type hash.
   */
  [[nodiscard]] constexpr id_type hash() const noexcept { return identifier; }

  /**
   * \brief Type name.
   * @return Type name.
   */
  [[nodiscard]] constexpr std::string_view name() const noexcept {
    return alias;
  }

 private:
  id_type seq;
  id_type identifier;
  std::string_view alias;
};

[[nodiscard]] inline constexpr bool operator==(const type_info &lhs,
                                               const type_info &rhs) noexcept {
  return lhs.hash() == rhs.hash();
}

[[nodiscard]] inline constexpr bool operator!=(const type_info &lhs,
                                               const type_info &rhs) noexcept {
  return !(lhs == rhs);
}

[[nodiscard]] constexpr bool operator<(const type_info &lhs,
                                       const type_info &rhs) noexcept {
  return lhs.index() < rhs.index();
}

[[nodiscard]] constexpr bool operator<=(const type_info &lhs,
                                        const type_info &rhs) noexcept {
  return !(rhs < lhs);
}

[[nodiscard]] constexpr bool operator>(const type_info &lhs,
                                       const type_info &rhs) noexcept {
  return rhs < lhs;
}

[[nodiscard]] constexpr bool operator>=(const type_info &lhs,
                                        const type_info &rhs) noexcept {
  return !(lhs < rhs);
}

/**
 * \brief 指定された型に関連付けられた型情報オブジェクトを返します
 *
 * @tparam Type 型情報オブジェクトを生成する型
 * @return 初期化された型情報の参照
 */
template <typename Type>
[[nodiscard]] const type_info &type_id() noexcept {
  if constexpr (std::is_same_v<
                    Type, std::remove_cv_t<std::remove_reference_t<Type>>>) {
    static type_info instance{std::in_place_type<Type>};
    return instance;
  } else {
    return type_id<std::remove_cv_t<std::remove_reference_t<Type>>>();
  }
}

/*! @copydoc type_id */
template <typename Type>
[[nodiscard]] const type_info &type_id(Type &&) noexcept {
  return type_id<std::remove_cv_t<std::remove_reference_t<Type>>>();
}

}  // namespace becs

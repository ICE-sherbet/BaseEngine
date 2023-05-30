// @BecsIterator.h
// @brief
// @author ICE
// @date 2023/03/16
//
// @details

#pragma once
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace becs {
template <typename Type>
struct input_iterator_pointer final {
  using value_type = Type;
  using pointer = Type *;
  using reference = Type &;

  /**
   * @brief moveにより構築する
   * @param val プロキシオブジェクトの初期化に使用する値
   */
  constexpr input_iterator_pointer(value_type &&val) noexcept(
      std::is_nothrow_move_constructible_v<value_type>)
      : value{std::move(val)} {}

  /**
   * @brief wrap された値にアクセスするためのアクセス修飾子
   * @return wrap された値のポインタ
   */
  [[nodiscard]] constexpr pointer operator->() noexcept {
    return std::addressof(value);
  }

  /**
   * @brief wrap された値にアクセスするための参照演算子
   * @return wrap された値の参照
   */
  [[nodiscard]] constexpr reference operator*() noexcept { return value; }

 private:
  Type value;
};

/**
 * @brief Plain iota iterator (waiting for C++20).
 * @tparam Type Value type.
 */
template <typename Type>
class iota_iterator final {
  static_assert(std::is_integral_v<Type>, "Not an integral type");

 public:
  using value_type = Type;
  using pointer = void;
  using reference = value_type;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;

  constexpr iota_iterator() noexcept : current{} {}

  constexpr iota_iterator(const value_type init) noexcept : current{init} {}

  constexpr iota_iterator &operator++() noexcept { return ++current, *this; }

  constexpr iota_iterator operator++(int) noexcept {
    iota_iterator orig = *this;
    return ++(*this), orig;
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return current;
  }

 private:
  value_type current;
};

template <typename Type>
[[nodiscard]] constexpr bool operator==(
    const iota_iterator<Type> &lhs, const iota_iterator<Type> &rhs) noexcept {
  return *lhs == *rhs;
}

template <typename Type>
[[nodiscard]] constexpr bool operator!=(
    const iota_iterator<Type> &lhs, const iota_iterator<Type> &rhs) noexcept {
  return !(lhs == rhs);
}

/**
 * \brief
 * イテレータのペアから反復可能なオブジェクトを作成するためのユーティリティクラス
 * \tparam It イテレータ型
 * \tparam Sentinel センチネル型
 */
template <typename It, typename Sentinel = It>
struct iterable_adaptor final {
  using value_type = typename std::iterator_traits<It>::value_type;
  using iterator = It;
  using sentinel = Sentinel;

  constexpr iterable_adaptor() noexcept(
      std::is_nothrow_default_constructible_v<iterator>
          &&std::is_nothrow_default_constructible_v<sentinel>)
      : first{}, last{} {}

  /**
   * @brief イテレータのペアから反復可能なオブジェクトを生成する
   * @param from 先頭イテレータ
   * @param to 終了イテレータ
   */
  constexpr iterable_adaptor(iterator from, sentinel to) noexcept(
      std::is_nothrow_move_constructible_v<iterator>
          &&std::is_nothrow_move_constructible_v<sentinel>)
      : first{std::move(from)}, last{std::move(to)} {}

  /**
   * @brief 先頭要素を指すイテレータを取得する
   * @return 先頭要素へのイテレータ
   */
  [[nodiscard]] constexpr iterator begin() const noexcept { return first; }

  /**
   * @brief 末尾の次を指すイテレータを取得する
   * @return 最後尾要素の次を指すイテレータを返す
   */
  [[nodiscard]] constexpr sentinel end() const noexcept { return last; }

  /*! @copydoc begin */
  [[nodiscard]] constexpr iterator cbegin() const noexcept { return begin(); }

  /*! @copydoc end */
  [[nodiscard]] constexpr sentinel cend() const noexcept { return end(); }

 private:
  It first;
  Sentinel last;
};

}  // namespace becs
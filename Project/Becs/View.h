// @View.h
// @brief
// @author ICE
// @date 2023/03/09
//
// @details

#pragma once
#include "BecsIterator.h"
#include "EntityFwd.h"
#include "Traits/Traits.h"
#include "TypeTraits.h"

namespace becs {
namespace internal {

template <typename... Args, typename Type, std::size_t N>
auto filter_as_tuple(const std::array<const Type *, N> &filter) noexcept {
  return std::apply(
      [](const auto *...curr) {
        return std::make_tuple(static_cast<Args *>(
            const_cast<constness_as_t<Type, Args> *>(curr))...);
      },
      filter);
}

template <typename Type, std::size_t N>
[[nodiscard]] auto none_of(const std::array<const Type *, N> &filter,
                           const typename Type::entity_type elem) noexcept {
  return std::apply(
      [elem](const auto *...curr) { return (!curr->contains(elem) && ...); },
      filter);
}

template <typename Type, std::size_t Get, std::size_t Exclude>
class view_iterator final {
  using iterator_type = typename Type::const_iterator;

  [[nodiscard]] bool valid() const noexcept {
    return ((Get != 0u) || (*it != tombstone)) &&
           std::apply(
               [elem = *it](const auto *...curr) {
                 return (curr->contains(elem) && ...);
               },
               pools) &&
           none_of(filter, *it);
  }

 public:
  using value_type = typename iterator_type::value_type;
  using pointer = typename iterator_type::pointer;
  using reference = typename iterator_type::reference;
  using difference_type = typename iterator_type::difference_type;
  using iterator_category = std::forward_iterator_tag;

  constexpr view_iterator() noexcept : it{}, last{}, pools{}, filter{} {}

  view_iterator(iterator_type curr, iterator_type to,
                std::array<const Type *, Get> value,
                std::array<const Type *, Exclude> excl) noexcept
      : it{curr}, last{to}, pools{value}, filter{excl} {
    while (it != last && !valid()) {
      ++it;
    }
  }

  view_iterator &operator++() noexcept {
    while (++it != last && !valid()) {
    }
    return *this;
  }

  view_iterator operator++(int) noexcept {
    view_iterator orig = *this;
    return ++(*this), orig;
  }

  [[nodiscard]] pointer operator->() const noexcept { return &*it; }

  [[nodiscard]] reference operator*() const noexcept { return *operator->(); }

  template <typename LhsType, auto... LhsArgs, typename RhsType,
            auto... RhsArgs>
  friend constexpr bool operator==(
      const view_iterator<LhsType, LhsArgs...> &,
      const view_iterator<RhsType, RhsArgs...> &) noexcept;

 private:
  iterator_type it;
  iterator_type last;
  std::array<const Type *, Get> pools;
  std::array<const Type *, Exclude> filter;
};

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator==(
    const view_iterator<LhsType, LhsArgs...> &lhs,
    const view_iterator<RhsType, RhsArgs...> &rhs) noexcept {
  return lhs.it == rhs.it;
}

template <typename LhsType, auto... LhsArgs, typename RhsType, auto... RhsArgs>
[[nodiscard]] constexpr bool operator!=(
    const view_iterator<LhsType, LhsArgs...> &lhs,
    const view_iterator<RhsType, RhsArgs...> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename It, typename... Type>
struct extended_view_iterator final {
  using iterator_type = It;
  using difference_type = std::ptrdiff_t;
  using value_type =
      decltype(std::tuple_cat(std::make_tuple(*std::declval<It>()),
                              std::declval<Type>().get_as_tuple({})...));
  using pointer = input_iterator_pointer<value_type>;
  using reference = value_type;
  using iterator_category = std::input_iterator_tag;

  constexpr extended_view_iterator() : it{}, pools{} {}

  extended_view_iterator(It from, std::tuple<Type *...> value)
      : it{from}, pools{value} {}

  extended_view_iterator &operator++() noexcept { return ++it, *this; }

  extended_view_iterator operator++(int) noexcept {
    extended_view_iterator orig = *this;
    return ++(*this), orig;
  }

  [[nodiscard]] reference operator*() const noexcept {
    return std::apply(
        [elem = *it](auto *...curr) {
          return std::tuple_cat(std::make_tuple(elem),
                                curr->get_as_tuple(elem)...);
        },
        pools);
  }

  [[nodiscard]] pointer operator->() const noexcept { return operator*(); }

  [[nodiscard]] constexpr iterator_type base() const noexcept { return it; }

  template <typename... Lhs, typename... Rhs>
  friend bool constexpr operator==(
      const extended_view_iterator<Lhs...> &,
      const extended_view_iterator<Rhs...> &) noexcept;

 private:
  It it;
  std::tuple<Type *...> pools;
};

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator==(
    const extended_view_iterator<Lhs...> &lhs,
    const extended_view_iterator<Rhs...> &rhs) noexcept {
  return lhs.it == rhs.it;
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator!=(
    const extended_view_iterator<Lhs...> &lhs,
    const extended_view_iterator<Rhs...> &rhs) noexcept {
  return !(lhs == rhs);
}

}  // namespace internal

template <typename, typename, typename>
class basic_view;

/**
 * \brief
 * 特定のコンポーネントを持つエンティティのみを表示・反復処理するための機能を提供します。
 * 基本的には、エンティティのサブセットを取得し、それらのエンティティに対してイテレーションを行うことができます。
 * basic_viewは、高速なイテレーションを可能にします。
 * \tparam Get Entity が所持していて取得する必要のある複数のコンポーネント
 * \tparam Exclude Entity が所持していて除外したい複数のコンポーネント
 */
template <typename... Get, typename... Exclude>
class basic_view<get_t<Get...>, exclude_t<Exclude...>> {
  using underlying_type = std::common_type_t<typename Get::entity_type...,
                                             typename Exclude::entity_type...>;
  using basic_common_type = std::common_type_t<typename Get::base_type...,
                                               typename Exclude::base_type...>;

  template <typename, typename, typename>
  friend class basic_view;

  template <typename Type>
  static constexpr std::size_t index_of = type_list_index_v<
      std::remove_const_t<Type>,
      type_list<typename Get::value_type..., typename Exclude::value_type...>>;

  [[nodiscard]] auto opaque_check_set() const noexcept {
    std::array<const base_type *, sizeof...(Get) - 1u> other{};
    std::apply(
        [&other, pos = 0u, view = view](const auto *...curr) mutable {
          ((curr == view ? void() : void(other[pos++] = curr)), ...);
        },
        pools);
    return other;
  }

  template <std::size_t Curr, std::size_t Other, typename... Args>
  [[nodiscard]] auto dispatch_get(
      const std::tuple<underlying_type, Args...> &curr) const {
    if constexpr (Curr == Other) {
      return std::forward_as_tuple(std::get<Args>(curr)...);
    } else {
      return storage<Other>().get_as_tuple(std::get<0>(curr));
    }
  }

  template <std::size_t Curr, typename Func, std::size_t... Index>
  void each(Func &func, std::index_sequence<Index...>) const {
    for (const auto curr : storage<Curr>().each()) {
      if (const auto elem = std::get<0>(curr);
          ((sizeof...(Get) != 1u) || (elem != tombstone)) &&
          ((Curr == Index || storage<Index>().contains(elem)) && ...) &&
          internal::none_of(filter, elem)) {
        if constexpr (is_applicable_v<Func, decltype(std::tuple_cat(
                                                std::tuple<entity_type>{},
                                                std::declval<basic_view>().get(
                                                    {})))>) {
          std::apply(func, std::tuple_cat(std::make_tuple(elem),
                                          dispatch_get<Curr, Index>(curr)...));
        } else {
          std::apply(func, std::tuple_cat(dispatch_get<Curr, Index>(curr)...));
        }
      }
    }
  }

  template <typename Func, std::size_t... Index>
  void pick_and_each(Func &func, std::index_sequence<Index...> seq) const {
    ((&storage<Index>() == view ? each<Index>(func, seq) : void()), ...);
  }

 public:
  using entity_type = underlying_type;
  using size_type = std::size_t;
  using base_type = basic_common_type;
  using iterator = internal::view_iterator<base_type, sizeof...(Get) - 1u,
                                           sizeof...(Exclude)>;
  using iterable =
      iterable_adaptor<internal::extended_view_iterator<iterator, Get...>>;

  basic_view() noexcept : pools{}, filter{}, view{} {}

  /**
   * \brief 複数タイプのビューを構築する
   * @param value 反復する型のストレージ
   * @param excl ビューのフィルタリングに使用されるタイプのストレージ
   */
  basic_view(Get &...value, Exclude &...excl) noexcept
      : pools{&value...}, filter{&excl...}, view{std::get<0>(pools)} {
    ((view = value.size() < view->size() ? &value : view), ...);
  }

  /**
   * \brief 複数タイプのビューを構築する
   * @param value 反復する型のストレージ
   * @param excl ビューのフィルタリングに使用されるタイプのストレージ
   */
  basic_view(std::tuple<Get &...> value,
             std::tuple<Exclude &...> excl = {}) noexcept
      : basic_view{
            std::make_from_tuple<basic_view>(std::tuple_cat(value, excl))} {}

  /**
   * \brief 必要に応じて、内部のリーディング ビューを更新します
   * @return 新しく作成され、内部的に最適化されたビュー
   */
  [[nodiscard]] basic_view refresh() const noexcept {
    return std::apply(
        [](auto *...elem) { return basic_view{*elem...}; },
        std::tuple_cat(pools, internal::filter_as_tuple<Exclude...>(filter)));
  }

  /**
   * \brief ビューの先頭ストレージを返す
   * @return ビューの先頭ストレージ
   */
  [[nodiscard]] const base_type &handle() const noexcept { return *view; }

  /**
   * \brief 指定されたコンポーネント タイプのストレージを返します
   * @tparam Type ストレージがもつコンポーネント型
   * @return 指定されたコンポーネント タイプのストレージ
   */
  template <typename Type>
  [[nodiscard]] decltype(auto) storage() const noexcept {
    return storage<index_of<Type>>();
  }

  /**
   * \brief 指定された インデックス のストレージを返します
   * @tparam Index ストレージがもつ インデックス
   * @return 指定された インデックス のストレージ
   */
  template <std::size_t Index>
  [[nodiscard]] decltype(auto) storage() const noexcept {
    if constexpr (constexpr auto offset = sizeof...(Get); Index < offset) {
      return *std::get<Index>(pools);
    } else {
      return *std::get<Index - offset>(
          internal::filter_as_tuple<Exclude...>(filter));
    }
  }

  /**
   * \brief ビューによって反復されるエンティティの数を推測する
   * @return ビューによって反復されるエンティティの推定数
   */
  [[nodiscard]] size_type size_hint() const noexcept { return handle().size(); }

  /**
   * \brief 先頭の要素を指す Entityイテレータ を取得する
   * 空の場合は end と等しくなります。
   * @return 最初のEntityイテレータを
   */
  [[nodiscard]] iterator begin() const noexcept {
    return iterator{handle().begin(), handle().end(), opaque_check_set(),
                    filter};
  }

  /**
   * \brief 末尾の次を指す Entityイテレータ を取得する
   * @return 末尾の次を指す Entityイテレータ
   */
  [[nodiscard]] iterator end() const noexcept {
    return iterator{handle().end(), handle().end(), opaque_check_set(), filter};
  }

  /**
   * \brief ビューの最初のエンティティがあればそれを返します
   * @return size > 0 : 最初のエンティティ
   * else nullエンティティ
   */
  [[nodiscard]] entity_type front() const noexcept {
    const auto it = begin();
    return it != end() ? *it : null;
  }

  /**
   * \brief ビューの最後のエンティティがあればそれを返します
   * @return size > 0 : 最後のエンティティ
   * else nullエンティティ
   */
  [[nodiscard]] entity_type back() const noexcept {
    auto it = handle().rbegin();
    for (const auto last = handle().rend(); it != last && !contains(*it);
         ++it) {
    }
    return it == handle().rend() ? null : *it;
  }

  /**
   * \brief Entityを検索します
   * @param elem 有効な識別子
   * @return 指定した値が見つかった場合はその要素へのイテレータ、
   * そうでない場合は end へのイテレータ
   */
  [[nodiscard]] iterator find(const entity_type elem) const noexcept {
    return contains(elem) ? iterator{handle().find(elem), handle().end(),
                                     opaque_check_set(), filter}
                          : end();
  }

  /**
   * \brief 要素へのアクセス
   * @param elem 有効な識別子
   * @return 特定のエンティティに割り当てられたコンポーネント
   */
  [[nodiscard]] decltype(auto) operator[](const entity_type elem) const {
    return get(elem);
  }

  /**
   * \brief ビューが適切に初期化されているかどうかを確認します.
   * \return ビューが適切に初期化されている場合は true 、そうでない場合は false
   */
  [[nodiscard]] explicit operator bool() const noexcept {
    return view != nullptr;
  }

  /**
   * \brief 
   * \param elem 
   * \return 
   */
  [[nodiscard]] bool contains(const entity_type elem) const noexcept {
    return std::apply(
               [elem](const auto *...curr) {
                 return (curr->contains(elem) && ...);
               },
               pools) &&
           internal::none_of(filter, elem);
  }


  template <typename... Type>
  [[nodiscard]] decltype(auto) get(const entity_type elem) const {
    if constexpr (sizeof...(Type) == 0) {
      return std::apply(
          [elem](auto *...curr) {
            return std::tuple_cat(curr->get_as_tuple(elem)...);
          },
          pools);
    } else if constexpr (sizeof...(Type) == 1) {
      return (storage<index_of<Type>>().get(elem), ...);
    } else {
      return std::tuple_cat(storage<index_of<Type>>().get_as_tuple(elem)...);
    }
  }


  template <std::size_t First, std::size_t... Other>
  [[nodiscard]] decltype(auto) get(const entity_type elem) const {
    if constexpr (sizeof...(Other) == 0) {
      return storage<First>().get(elem);
    } else {
      return std::tuple_cat(storage<First>().get_as_tuple(elem),
                            storage<Other>().get_as_tuple(elem)...);
    }
  }

  /**
   * \brief エンティティとコンポーネントを反復し、
   * 指定された関数オブジェクトをそれらに適用します。
   * @tparam Func 関数オブジェクト型
   * @param func 有効な関数オブジェクト
   */
  template <typename Func>
  void each(Func func) const {
    pick_and_each(func, std::index_sequence_for<Get...>{});
  }

  /**
   * \brief ビューにアクセスでき、反復可能なオブジェクトを返します
   *
   * @return ビューを操作するための反復可能なオブジェクト
   */
  [[nodiscard]] iterable each() const noexcept {
    return {internal::extended_view_iterator{begin(), pools},
            internal::extended_view_iterator{end(), pools}};
  }

  /**
   * \brief 2つのビューを１つのビューに結合する
   * @tparam OGet 結合するビューのコンポーネントリスト
   * @tparam OExclude 結合するビューのフィルタ
   * @param other もう一つのビュー
   * @return あたらしいビュー
   */
  template <typename... OGet, typename... OExclude>
  [[nodiscard]] auto operator|(
      const basic_view<get_t<OGet...>, exclude_t<OExclude...>> &other)
      const noexcept {
    return std::apply(
        [](auto *...curr) {
          return basic_view<get_t<Get..., OGet...>,
                            exclude_t<Exclude..., OExclude...>>{*curr...};
        },
        std::tuple_cat(pools, other.pools,
                       internal::filter_as_tuple<Exclude...>(filter),
                       internal::filter_as_tuple<OExclude...>(other.filter)));
  }

 private:
  std::tuple<Get *...> pools;
  std::array<const base_type *, sizeof...(Exclude)> filter;
  const base_type *view;
};
/*
template <typename Get>
class basic_view<
    get_t<Get>, exclude_t<>,
    std::void_t<std::enable_if_t<!Get::traits_type::in_place_delete>>> {};
    */
}  // namespace becs
// @Registry.h
// @brief EntityやComponentを管理するクラス
// @author ICE
// @date 2023/03/02
//
// @details

#pragma once
#include <vector>

#include "Algorithm.h"
#include "BecsIterator.h"
#include "Entity.h"
#include "SignalHandlerMixin.h"
#include "SparseSet.h"
#include "Storage.h"
#include "Traits/Traits.h"
#include "TypeInfo.h"
#include "TypeTraits.h"
#include "View.h"
#include <algorithm>

namespace becs {

namespace internal {

template <typename It>
class registry_storage_iterator final {
  template <typename Other>
  friend class registry_storage_iterator;

  using mapped_type =
      std::remove_reference_t<decltype(std::declval<It>()->second)>;

 public:
  using value_type =
      std::pair<id_type, constness_as_t<typename mapped_type::element_type,
                                        mapped_type> &>;
  using pointer = input_iterator_pointer<value_type>;
  using reference = value_type;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;

  constexpr registry_storage_iterator() noexcept : it{} {}

  constexpr registry_storage_iterator(It iter) noexcept : it{iter} {}

  template <typename Other,
            typename = std::enable_if_t<!std::is_same_v<It, Other> &&
                                        std::is_constructible_v<It, Other>>>
  constexpr registry_storage_iterator(
      const registry_storage_iterator<Other> &other) noexcept
      : registry_storage_iterator{other.it} {}

  constexpr registry_storage_iterator &operator++() noexcept {
    return ++it, *this;
  }

  constexpr registry_storage_iterator operator++(int) noexcept {
    registry_storage_iterator orig = *this;
    return ++(*this), orig;
  }

  constexpr registry_storage_iterator &operator--() noexcept {
    return --it, *this;
  }

  constexpr registry_storage_iterator operator--(int) noexcept {
    registry_storage_iterator orig = *this;
    return operator--(), orig;
  }

  constexpr registry_storage_iterator &operator+=(
      const difference_type value) noexcept {
    it += value;
    return *this;
  }

  constexpr registry_storage_iterator operator+(
      const difference_type value) const noexcept {
    registry_storage_iterator copy = *this;
    return (copy += value);
  }

  constexpr registry_storage_iterator &operator-=(
      const difference_type value) noexcept {
    return (*this += -value);
  }

  constexpr registry_storage_iterator operator-(
      const difference_type value) const noexcept {
    return (*this + -value);
  }

  [[nodiscard]] constexpr reference operator[](
      const difference_type value) const noexcept {
    return {it[value].first, *it[value].second};
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return {it->first, *it->second};
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
    return operator*();
  }

  template <typename Lhs, typename Rhs>
  friend constexpr std::ptrdiff_t operator-(
      const registry_storage_iterator<Lhs> &,
      const registry_storage_iterator<Rhs> &) noexcept;

  template <typename Lhs, typename Rhs>
  friend constexpr bool operator==(
      const registry_storage_iterator<Lhs> &,
      const registry_storage_iterator<Rhs> &) noexcept;

  template <typename Lhs, typename Rhs>
  friend constexpr bool operator<(
      const registry_storage_iterator<Lhs> &,
      const registry_storage_iterator<Rhs> &) noexcept;

 private:
  It it;
};

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr std::ptrdiff_t operator-(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return lhs.it - rhs.it;
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator==(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return lhs.it == rhs.it;
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator!=(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator<(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return lhs.it < rhs.it;
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator>(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return rhs < lhs;
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator<=(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return !(lhs > rhs);
}

template <typename Lhs, typename Rhs>
[[nodiscard]] constexpr bool operator>=(
    const registry_storage_iterator<Lhs> &lhs,
    const registry_storage_iterator<Rhs> &rhs) noexcept {
  return !(lhs < rhs);
}

}  // namespace internal

/**
 * \brief EntityやComponentの管理を行う
 * シグナルやクエリなどを発行できる
 * \tparam Entity
 */
template <typename Entity, typename Allocator>
class BasicRegistry {
public:
  using basic_common_type = basic_sparse_set<Entity, Allocator>;

  using alloc_traits = std::allocator_traits<Allocator>;
  static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>,
                "Invalid value type");
  using container_type =
      dense_map<IdType, std::shared_ptr<basic_common_type>, std::identity,
                std::equal_to<>,
                typename alloc_traits::template rebind_alloc<std::pair<
                    const IdType, std::shared_ptr<basic_common_type>>>>;

  template <typename Type>
  using storage_for_type =
      typename storage_for<Type, Entity,
                           typename alloc_traits::template rebind_alloc<
                               std::remove_const_t<Type>>>::type;

  template <typename...>
  struct group_handler;

  template <typename... Exclude, typename... Get, typename... Owned>
  struct group_handler<exclude_t<Exclude...>, get_t<Get...>, Owned...> {
    static_assert(
        !std::disjunction_v<std::bool_constant<
            storage_for_type<Owned>::traits_type::in_place_delete>...>,
        "Groups do not support in-place delete");
    using value_type = std::conditional_t<sizeof...(Owned) == 0,
                                          basic_common_type, std::size_t>;
    value_type current{};

    template <typename... Args>
    group_handler(Args &&...args) : current{std::forward<Args>(args)...} {}

    template <typename Type>
    void maybe_valid_if(BasicRegistry &owner, const Entity elem) {
      [[maybe_unused]] const auto cpools =
          std::forward_as_tuple(owner.storage<Owned>()...);

      const auto is_valid =
          ((std::is_same_v<Type, Owned> ||
            std::get<storage_for_type<Owned> &>(cpools).contains(elem)) &&
           ...) &&
          ((std::is_same_v<Type, Get> || owner.storage<Get>().contains(elem)) &&
           ...) &&
          ((std::is_same_v<Type, Exclude> ||
            !owner.storage<Exclude>().contains(elem)) &&
           ...);

      if constexpr (sizeof...(Owned) == 0) {
        if (is_valid && !current.contains(elem)) {
          current.push(elem);
        }
      } else {
        if (is_valid && !(std::get<0>(cpools).index(elem) < current)) {
          const auto pos = current++;
          (std::get<storage_for_type<Owned> &>(cpools).swap_elements(
               std::get<storage_for_type<Owned> &>(cpools).data()[pos], elem),
           ...);
        }
      }
    }

    void discard_if([[maybe_unused]] BasicRegistry &owner, const Entity elem) {
      if constexpr (sizeof...(Owned) == 0) {
        current.remove(elem);
      } else {
        if (const auto cpools =
                std::forward_as_tuple(owner.storage<Owned>()...);
            std::get<0>(cpools).contains(elem) &&
            (std::get<0>(cpools).index(elem) < current)) {
          const auto pos = --current;
          (std::get<storage_for_type<Owned> &>(cpools).swap_elements(
               std::get<storage_for_type<Owned> &>(cpools).data()[pos], elem),
           ...);
        }
      }
    }
  };

  struct group_data {
    std::size_t size;
    std::shared_ptr<void> group;
    bool (*owned)(const id_type) noexcept;
    bool (*get)(const id_type) noexcept;
    bool (*exclude)(const id_type) noexcept;
  };

  template <typename Type>
  [[nodiscard]] auto &assure(const id_type id = type_hash<Type>::value()) {
    static_assert(std::is_same_v<Type, std::decay_t<Type>>,
                  "Non-decayed types not allowed");
    auto &cpool = pools[id];

    if (!cpool) {
      using alloc_type =
          typename storage_for_type<std::remove_const_t<Type>>::allocator_type;

      if constexpr (std::is_same_v<Type, void> &&
                    !std::is_constructible_v<alloc_type, allocator_type>) {
        cpool =
            std::allocate_shared<storage_for_type<std::remove_const_t<Type>>>(
                get_allocator(), alloc_type{});
      } else {
        cpool =
            std::allocate_shared<storage_for_type<std::remove_const_t<Type>>>(
                get_allocator(), get_allocator());
      }

      cpool->bind(forward_as_any(*this));
    }

    return static_cast<storage_for_type<Type> &>(*cpool);
  }

  template <typename Type>
  [[nodiscard]] const auto &assure(
      const id_type id = type_hash<Type>::value()) const {
    static_assert(std::is_same_v<Type, std::decay_t<Type>>,
                  "Non-decayed types not allowed");

    if (const auto it = pools.find(id); it != pools.cend()) {
      return static_cast<const storage_for_type<Type> &>(*it->second);
    }

    static storage_for_type<Type> placeholder{};
    return placeholder;
  }
  template <typename AnyType>
  [[nodiscard]] std::any forward_as_any(AnyType &&value) {
    return std::any{std::ref(value)};
  }
  void rebind() {
    std::any an = std::ref(*this);
    entities->bind(an);
    for (auto &&curr : pools) {
      curr.second->bind(an);
    }
  }

 public:
  using traits_type = BecsTraits<Entity>;
  using allocator_type = Allocator;
  using entity_type = Entity;
  using version_type = typename traits_type::VersionType;
  using size_type = std::size_t;
  using base_type = basic_common_type;

  BasicRegistry() : BasicRegistry{allocator_type{}} {}

  explicit BasicRegistry(const allocator_type &allocator)
      : BasicRegistry{0u, allocator} {}

  BasicRegistry(const size_type count,
                const allocator_type &allocator = allocator_type{})
      : pools{allocator},
        entities{std::allocate_shared<storage_for_type<entity_type>>(
            allocator, allocator)},
        groups_{allocator} {
    pools.reserve(count);
    rebind();
  }

  BasicRegistry(BasicRegistry &&other) noexcept
      : pools{std::move(other.pools)},
        entities{std::move(other.entities)},
        groups_{std::move(other.groups_)} {
    rebind();
  }

  BasicRegistry &operator=(BasicRegistry &&other) noexcept {
    pools = std::move(other.pools);
    entities = std::move(other.entities);
    groups_ = std::move(other.groups_);

    rebind();

    return *this;
  }

  /**
   * @brief アロケータを返す
   * @return .
   */
  [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
    return pools.get_allocator();
  }

  /**
   * \brief レジストリにアクセスし反復可能なオブジェクトを返します。
   * 要素はIDと参照のペアで構成されている
   * \return 反復可能なオブジェクト
   */
  [[nodiscard]] auto storage() noexcept {
    return iterable_adaptor{internal::registry_storage_iterator{pools.begin()},
                            internal::registry_storage_iterator{pools.end()}};
  }

  /*! @copydoc storage */
  [[nodiscard]] auto storage() const noexcept {
    return iterable_adaptor{internal::registry_storage_iterator{pools.cbegin()},
                            internal::registry_storage_iterator{pools.cend()}};
  }

  /**
   * \brief Idからストレージに関連付けされた要素を返す
   * \param id レジストリ内のストレージをマップするための要素
   * \return ストレージが存在する場合はそのポインタ、そうでない場合は null
   * ポインタ。
   */
  [[nodiscard]] base_type *storage(const id_type id) {
    return const_cast<base_type *>(std::as_const(*this).storage(id));
  }

  /**
   * \brief Idからストレージに関連付けされた要素を返す
   * \param id レジストリ内のストレージをマップするための要素
   * \return ストレージが存在する場合はそのポインタ、そうでない場合は null
   * ポインタ。
   */
  [[nodiscard]] const base_type *storage(const id_type id) const {
    const auto it = pools.find(id);
    return it == pools.cend() ? nullptr : it->second.get();
  }

  /**
   * @brief 指定のコンポーネント型を持ったストレージを返します
   * @tparam Type ストレージを返すコンポーネントのタイプ
   * @param id レジストリ内のストレージをマップするために使用される名前.
   * @return 指定のコンポーネント型を持ったストレージ
   */
  template <typename Type>
  decltype(auto) storage(const id_type id = type_hash<Type>::value()) {
    return assure<Type>(id);
  }

  /**
   * \brief 指定されたコンポーネントタイプのストレージを返します。
   * \tparam Type コンポーネント型
   * \param id
   * レジストリ内のストレージをマップするために使用されるオプションのId \return
   * 指定されたコンポーネント タイプのストレージ
   */
  template <typename Type>
  decltype(auto) storage(const id_type id = type_hash<Type>::value()) const {
    return assure<Type>(id);
  }

  /**
   * @brief 生成されている Entity の個数を返す
   * @return 生成されている Entity の数
   */
  [[nodiscard]] size_type size() const noexcept { return entities->size(); }

  /**
   * @brief 有効な Entity の数を返す
   * @return 有効な Entity 数
   */
  [[nodiscard]] size_type alive() const { return entities->in_use(); }

  /**
   * @brief レジストリの容量を増やす
   * @param cap レジストリ容量
   */
  void reserve(const size_type cap) { entities->reserve(cap); }

  /**
   * \brief Entity用に割り当てられているサイズを返す
   * \return レジストリ容量
   */
  [[nodiscard]] size_type capacity() const noexcept {
    return entities->capacity();
  }

  /**
   * @brief Entityが空であるか。
   * @return 空であれば true。でなければ false
   */
  [[nodiscard]] bool empty() const { return !alive(); }

  /**
   * \brief レジストリへの直接アクセス
   * 返されるポインタの有効範囲は "data()"-"data() + size()"
   * 有効範囲であれば初期化されているので未定義動作とはならない。
   * \return Entity配列のポインタ
   */
  [[nodiscard]] const entity_type *data() const noexcept {
    return entities->data();
  }

  /**
   * \brief 識別子が有効なエンティティを参照しているかどうかを確認する
   * \param elem 識別子
   * \return 有効であれば true、無効であれば false
   */
  [[nodiscard]] bool valid(const entity_type elem) const {
    return entities->contains(elem);
  }

  /**
   * \brief Entityを新しく生成するか、もしくは破棄済みのものをリサイクルします。
   * \return 有効な識別子
   */
  [[nodiscard]] entity_type create() { return entities->spawn(); }

  /**
   * \copybrief create
   * ヒントを利用し Entity を生成する
   *
   * @param 指定の識別子のヒント
   * @return 有効な識別子
   */
  [[nodiscard]] entity_type create(const entity_type hint) {
    return entities->spawn(hint);
  }

  /**
   * \brief 生成リストの範囲から生成を行う
   * \tparam It イテレータ型
   * \param first 生成範囲の先頭要素
   * \param last 生成範囲の末尾の次要素
   */
  template <typename It>
  void create(It first, It last) {
    entities->spawn(std::move(first), std::move(last));
  }

  /**
   * \brief 識別子を解放します。
   * 解放することでバージョン(世代)が上がり、再利用可能となります。
   * \warning
   * 無効なEntityを指定すると未定義動作を起こします。
   * \param elem 有効な識別子
   * \return 変更後のバージョン
   */
  version_type release(const entity_type elem) {
    entities->erase(elem);
    return entities->current(elem);
  }

  /**
   * \brief 識別子を解放します。
   * 暗黙的に生成されたバージョンの代わりに、
   * 指定バージョンまたは指定バージョンに最も近い有効なバージョンが使用されます。
   * \param elem 有効な識別子
   * \param version 指定のバージョン
   * \return 変更後のバージョン
   */
  version_type release(const entity_type elem, const version_type version) {
    release(elem);
    const auto vers = static_cast<version_type>(
        version + (version == traits_type::to_version(tombstone)));
    return vers;
  }

  /**
   * \brief 範囲を指定してEntityの解放を行う
   * \tparam It Entityのイテレーター型
   * \param first 範囲の先頭要素のイテレーター
   * \param last 範囲の末尾要素のイテレーター
   */
  template <typename It>
  void release(It first, It last) {
    entities->erase(std::move(first), std::move(last));
  }

  /**
   * @brief 識別子を破棄し、解放する
   *
   * @param elem 有効な識別子
   * @return リサイクルされたEntityのバージョン
   */
  version_type destroy(const entity_type elem) {
    return destroy(
        elem, static_cast<version_type>(traits_type::to_version(elem) + 1u));
  }

  /**
   * \brief 識別子を破棄し、解放します。
   * 暗黙的に生成されたバージョンの代わりに、
   * 指定バージョンまたは指定バージョンに最も近い有効なバージョンが使用されます。
   * \param elem 有効な識別子
   * \param version 指定のバージョン
   * \return 変更後のバージョン
   */
  version_type destroy(const entity_type elem, const version_type version) {
    for (auto &[_, pool] : pools) {
      pool->remove(elem);
    }
    return release(elem, version);
  }

  /**
   * \brief 範囲を指定してEntityの破棄、解放を行う
   * \tparam It Entityのイテレーター型
   * \param first 範囲の先頭要素のイテレーター
   * \param last 範囲の末尾要素のイテレーター
   */
  template <typename It>
  void destroy(It first, It last) {
    const auto len = entities->pack(first, last);
    auto from = entities->each().cbegin().base();
    const auto to = from + len;

    for (size_type pos = pools.size(); pos; --pos) {
      pools.begin()[pos - 1u].second->remove(from, to);
    }

    entities->erase(from, to);
  }

  /**
   * \brief 指定のコンポーネントを Entity に割り当てます。
   *
   * \tparam Type 指定のコンポーネント型
   * \tparam Args コンポーネントを構築するのに必要な引数
   * \param elem 有効な識別子
   * \param args コンポーネントのコンストラクタの引数
   * \return 割り当てられたコンポーネントの参照
   */
  template <typename Type, typename... Args>
  decltype(auto) emplace(const entity_type elem, Args &&...args) {
    return assure<Type>().emplace(elem, std::forward<Args>(args)...);
  }

  /**
   * \brief Entity の指定のコンポーネントを割り当てるか、置き換えます。
   * \tparam Type 指定のコンポーネント型
   * \tparam Args コンポーネントのコンストラクタの引数型
   * \param elem 有効な識別子
   * \param args コンポーネントのコンストラクタの引数
   * \return 指定のコンポーネントの参照
   */
  template <typename Type, typename... Args>
  decltype(auto) emplace_or_replace(const entity_type elem, Args &&...args) {
    if (auto &cpool = assure<Type>(); cpool.contains(elem)) {
      return cpool.patch(elem, [... args = std::move(args)](auto &...curr) {
        ((curr = Type{args...}), ...);
      });

    } else {
      return cpool.emplace(elem, std::forward<Args>(args)...);
    }
  }

  /**
   * \brief 登録されている Entity
   * の指定コンポーネントに対してのパッチ処理で更新を行う \tparam Type
   * 指定のコンポーネント型 \tparam Func 更新関数型 \param elem 有効な識別子
   * \param func 更新関数
   * \return コンポーネントの参照
   */
  template <typename Type, typename... Func>
  decltype(auto) patch(const entity_type elem, Func &&...func) {
    return assure<Type>().patch(elem, std::forward<Func>(func)...);
  }

  /**
   * \brief 登録されている Entity の指定コンポーネントに対しての置き換えを行う
   * \tparam Type 指定のコンポーネント型
   * \tparam Args コンポーネントを構築するための引数型
   * \param elem 有効な識別子
   * \param args コンポーネントのコンストラクタの引数
   * \return コンポーネントの参照
   */
  template <typename Type, typename... Args>
  decltype(auto) replace(const entity_type elem, Args &&...args) {
    return patch<Type>(elem, [... args = std::move(args)](auto &...curr) {
      ((curr = Type{std::forward<Args>(args)...}), ...);
    });
  }

  /**
   * \brief Entity から指定コンポーネントを削除する。
   * 削除する際に Entity
   * に指定コンポーネントがあるかを判定してから削除を行うので安全 \tparam Type
   * 削除を行うコンポーネント群の先頭 \tparam Other
   * 削除を行うコンポーネント群の先頭以外 \param elem 有効な識別子 \return
   * 削除されたコンポーネント数
   */
  template <typename Type, typename... Other>
  size_type remove(const entity_type elem) {
    return (assure<Type>().remove(elem) + ... + assure<Other>().remove(elem));
  }

  /**
   * \brief Entity の領域を渡し指定コンポーネントを削除する。
   * \tparam Type 削除を行うコンポーネント群の先頭
   * \tparam Other 削除を行うコンポーネント群の先頭以外
   * \tparam It 範囲イテレーター型
   * \param first 先頭要素のイテレーター
   * \param last 末尾要素のイテレーター
   * \return 削除されたコンポーネント数
   */
  template <typename Type, typename... Other, typename It>
  size_type remove(It first, It last) {
    size_type count{};

    if constexpr (sizeof...(Other) == 0u) {
      count += assure<Type>().remove(std::move(first), std::move(last));
    } else if constexpr (std::is_same_v<It, typename base_type::iterator>) {
      constexpr size_type len = sizeof...(Other) + 1u;
      base_type *cpools[len]{&assure<Type>(), &assure<Other>()...};

      for (size_type pos{}; pos < len; ++pos) {
        if (cpools[pos]->data() == first.data()) {
          std::swap(cpools[pos], cpools[len - 1u]);
        }

        count += cpools[pos]->remove(first, last);
      }
    } else {
      for (auto cpools =
               std::forward_as_tuple(assure<Type>(), assure<Other>()...);
           first != last; ++first) {
        count += std::apply(
            [elem = *first](auto &...curr) {
              return (curr.remove(elem) + ... + 0u);
            },
            cpools);
      }
    }

    return count;
  }

  /**
   * \brief Entity から 指定コンポーネント群を削除する
   * \tparam Type 削除を行うコンポーネント群の先頭
   * \tparam Other 削除を行うコンポーネント群の先頭以外
   * \param elem 有効な識別子
   */
  template <typename Type, typename... Other>
  void erase(const entity_type elem) {
    (assure<Type>().erase(elem), (assure<Other>().erase(elem), ...));
  }

  /**
   * \brief Entity の領域を渡し指定コンポーネントを削除する。
   * \tparam Type 削除を行うコンポーネント群の先頭
   * \tparam Other 削除を行うコンポーネント群の先頭以外
   * \tparam It 範囲イテレーター型
   * \param first 先頭要素のイテレーター
   * \param last 末尾要素のイテレーター
   * \return 削除されたコンポーネント数
   */
  template <typename Type, typename... Other, typename It>
  void erase(It first, It last) {
    if constexpr (sizeof...(Other) == 0u) {
      assure<Type>().erase(std::move(first), std::move(last));
    } else if constexpr (std::is_same_v<It, typename base_type::iterator>) {
      constexpr size_type len = sizeof...(Other) + 1u;
      base_type *cpools[len]{&assure<Type>(), &assure<Other>()...};

      for (size_type pos{}; pos < len; ++pos) {
        if (cpools[pos]->data() == first.data()) {
          std::swap(cpools[pos], cpools[len - 1u]);
        }

        cpools[pos]->erase(first, last);
      }
    } else {
      for (auto cpools =
               std::forward_as_tuple(assure<Type>(), assure<Other>()...);
           first != last; ++first) {
        std::apply([elem = *first](auto &...curr) { (curr.erase(elem), ...); },
                   cpools);
      }
    }
  }

  /**
   * \brief Registryからtombstonesを全て削除し、指定のコンポーネントのみ削除可能
   * \tparam Type 0つ以上のコンポーネント型
   */
  template <typename... Type>
  void compact() {
    if constexpr (sizeof...(Type) == 0) {
      for (auto &&curr : pools) {
        curr.second->compact();
      }
    } else {
      (assure<Type>().compact(), ...);
    }
  }

  /**
   * \brief Entity
   * が指定のコンポーネントを所持しているかどうかのチェックを行う。
   * \tparam Type 指定のコンポーネント型
   * \param elem 有効な識別子
   * \return コンポーネントを所持しているか
   */
  template <typename... Type>
  [[nodiscard]] bool all_of(const entity_type elem) const {
    return (assure<std::remove_const_t<Type>>().contains(elem) && ...);
  }

  /**
   * \brief Entity が少なくとも
   * 指定する1つ以上のコンポーネントを所持しているかどうかをチェックする \tparam
   * Type 指定のコンポーネント型 \param elem 有効な識別子 \return
   * 1つ以上のコンポーネントを所持しているか
   */
  template <typename... Type>
  [[nodiscard]] bool any_of(const entity_type elem) const {
    return (assure<std::remove_const_t<Type>>().contains(elem) || ...);
  }

  /**
   * \brief Entity への指定されたコンポーネントへの参照を返します。
   * \tparam Type 指定コンポーネント型
   * \param elem 有効な識別子
   * \return Entity が所持するコンポーネントへの参照
   */
  template <typename... Type>
  [[nodiscard]] decltype(auto) get(const entity_type elem) const {
    if constexpr (sizeof...(Type) == 1u) {
      return (assure<std::remove_const_t<Type>>().get(elem), ...);
    } else {
      return std::forward_as_tuple(get<Type>(elem)...);
    }
  }

  /*! @copydoc get */
  template <typename... Type>
  [[nodiscard]] decltype(auto) get(const entity_type elem) {
    if constexpr (sizeof...(Type) == 1u) {
      return (const_cast<Type &>(std::as_const(*this).template get<Type>(elem)),
              ...);
    } else {
      return std::forward_as_tuple(get<Type>(elem)...);
    }
  }

  // TODO optionalに変更するか検討を行う
  /**
   * \brief Entityが所持する指定のコンポーネントのポインタを返します。
   * \tparam Type コンポーネント型
   * \param elem 有効な識別子
   * \return コンポーネントのポインタ、なければnullポインタ
   */
  template <typename... Type>
  [[nodiscard]] auto try_get([[maybe_unused]] const entity_type elem) const {
    if constexpr (sizeof...(Type) == 1) {
      const auto &cpool = assure<std::remove_const_t<Type>...>();
      return cpool.contains(elem) ? std::addressof(cpool.get(elem)) : nullptr;
    } else {
      return std::make_tuple(try_get<Type>(elem)...);
    }
  }

  /*! @copydoc try_get */
  template <typename... Type>
  [[nodiscard]] auto try_get([[maybe_unused]] const entity_type elem) {
    if constexpr (sizeof...(Type) == 1) {
      return (
          const_cast<Type *>(std::as_const(*this).template try_get<Type>(elem)),
          ...);
    } else {
      return std::make_tuple(try_get<Type>(elem)...);
    }
  }

  template <typename Component, typename Compare, typename Sort = std_sort,
            typename... Args>
  void sort(Compare compare, Sort algo = Sort{}, Args &&...args) {
    auto &cpool = assure<Component>();

    if constexpr (std::is_invocable_v<Compare, decltype(cpool.get({})),
                                      decltype(cpool.get({}))>) {
      auto comp = [&cpool, compare = std::move(compare)](const auto lhs,
                                                         const auto rhs) {
        return compare(std::as_const(cpool.get(lhs)),
                       std::as_const(cpool.get(rhs)));
      };
      cpool.sort(std::move(comp), std::move(algo), std::forward<Args>(args)...);
    } else {
      cpool.sort(std::move(compare), std::move(algo),
                 std::forward<Args>(args)...);
    }
  }

  /**
   * \brief 指定のコンポーネントを全て削除する
   * \tparam Type 指定のコンポーネント型
   */
  template <typename... Type>
  void clear() {
    if constexpr (sizeof...(Type) == 0) {
      for (size_type pos = pools.size(); pos; --pos) {
        pools.begin()[pos - 1u].second->clear();
      }

      auto iterable = entities->each();
      entities->erase(iterable.begin().base(), iterable.end().base());
    } else {
      (assure<Type>().clear(), ...);
    }
  }

  /**
   * \brief 有効な全てのEntityに繰り返し処理を行う
   * 反復中に作成されたエンティティが返されるかどうかは定義していません。
   * \tparam Func 関数型
   * \param func 有効な全てのEntityにたいして行う関数オブジェクト
   */
  template <typename Func>
  void each(Func func) const {
    for (auto [elem] : entities->each()) {
      func(elem);
    }
  }

  template <typename Type>
  [[nodiscard]] auto on_construct() {
    return assure<Type>().on_construct();
  }

  template <typename Type>
  [[nodiscard]] auto on_update() {
    return assure<Type>().on_update();
  }

  template <typename Type>
  [[nodiscard]] auto on_destroy() {
    return assure<Type>().on_destroy();
  }

  /**
   * \brief 指定されたコンポーネントのビューを返します。
   * ビューはオンザフライで生成し、ストレージと共有される
   * \tparam Type ビューを構築するコンポーネント群の先頭型
   * \tparam Other ビューを構築するコンポーネント群のその他
   * \tparam Exclude ビューから除外するコンポーネント群
   * \return 生成されたビュー
   */
  template <typename Type, typename... Other, typename... Exclude>
  [[nodiscard]] basic_view<
      get_t<storage_for_type<const Type>, storage_for_type<const Other>...>,
      exclude_t<storage_for_type<const Exclude>...>>
  view(exclude_t<Exclude...> = {}) const {
    return {assure<std::remove_const_t<Type>>(),
            assure<std::remove_const_t<Other>>()...,
            assure<std::remove_const_t<Exclude>>()...};
  }
  /*! @copydoc view */
  template <typename Type, typename... Other, typename... Exclude>
  [[nodiscard]] basic_view<
      get_t<storage_for_type<Type>, storage_for_type<Other>...>,
      exclude_t<storage_for_type<Exclude>...>>
  view(exclude_t<Exclude...> = {}) {
    return {assure<std::remove_const_t<Type>>(),
            assure<std::remove_const_t<Other>>()...,
            assure<std::remove_const_t<Exclude>>()...};
  }

  /**
   * \brief Entityに付いているコンポーネントを指定先にコピーする。\n
   * コピー先に同じコンポーネントがある場合は上書きされます。\n
   * コピー先にないStorageは新規に作成されます。
   * \tparam Exclude コピーから除外するコンポーネント群
   * \param src_entity コピー元のEntity
   * \param dst コピー先のレジストリ
   * \param dst_entity コピー先のEntity
   */
  template <typename... Exclude>
  void copy_to(Entity src_entity,
                              BasicRegistry &dst, Entity dst_entity) {
    constexpr std::array<uint32_t, sizeof...(Exclude)> exclude_args = {
        type_hash<Exclude>::value()...};

  	auto &dst_pool = dst.pools;
    for (auto &&[id, entity_map] : storage()) {
      bool skip = false;
      for (auto exclude : exclude_args) {
        if (exclude == id) skip = true;
      }
      if (skip) continue;
      auto &storage = entity_map;
      auto &dst_storage = dst_pool[id];

      if (storage.contains(src_entity)) {
        storage.copy(src_entity, dst_storage, dst_entity);
      }
    }
  }

 private:
  container_type pools;

  std::shared_ptr<storage_for_type<entity_type>> entities;
  std::vector<group_data,
              typename alloc_traits::template rebind_alloc<group_data>>
      groups_;
};

}  // namespace becs

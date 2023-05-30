// @SparseSet.h
// @brief 散在して存在してるデータの集合を扱う
// @author ICE
// @date 2023/03/08
//
// @details

#pragma once
#include <any>
#include <memory>
#include <vector>

#include "EntityFwd.h"
#include "Traits/Traits.h"
#include "TypeInfo.h"
namespace becs {

namespace internal {

template <typename Container>
struct sparse_set_iterator final {
  using value_type = typename Container::value_type;
  using pointer = typename Container::const_pointer;
  using reference = typename Container::const_reference;
  using difference_type = typename Container::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  constexpr sparse_set_iterator() noexcept : packed{}, offset{} {}

  constexpr sparse_set_iterator(const Container &ref,
                                const difference_type idx) noexcept
      : packed{std::addressof(ref)}, offset{idx} {}

  constexpr sparse_set_iterator &operator++() noexcept {
    return --offset, *this;
  }

  constexpr sparse_set_iterator operator++(int) noexcept {
    sparse_set_iterator orig = *this;
    return ++(*this), orig;
  }

  constexpr sparse_set_iterator &operator--() noexcept {
    return ++offset, *this;
  }

  constexpr sparse_set_iterator operator--(int) noexcept {
    sparse_set_iterator orig = *this;
    return operator--(), orig;
  }

  constexpr sparse_set_iterator &operator+=(
      const difference_type value) noexcept {
    offset -= value;
    return *this;
  }

  constexpr sparse_set_iterator operator+(
      const difference_type value) const noexcept {
    sparse_set_iterator copy = *this;
    return (copy += value);
  }

  constexpr sparse_set_iterator &operator-=(
      const difference_type value) noexcept {
    return (*this += -value);
  }

  constexpr sparse_set_iterator operator-(
      const difference_type value) const noexcept {
    return (*this + -value);
  }

  [[nodiscard]] constexpr reference operator[](
      const difference_type value) const noexcept {
    return packed->data()[index() - value];
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
    return packed->data() + index();
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return *operator->();
  }

  [[nodiscard]] constexpr pointer data() const noexcept {
    return packed ? packed->data() : nullptr;
  }

  [[nodiscard]] constexpr difference_type index() const noexcept {
    return offset - 1;
  }

 private:
  const Container *packed;
  difference_type offset;
};

template <typename Container>
[[nodiscard]] constexpr std::ptrdiff_t operator-(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return rhs.index() - lhs.index();
}

template <typename Container>
[[nodiscard]] constexpr bool operator==(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return lhs.index() == rhs.index();
}

template <typename Container>
[[nodiscard]] constexpr bool operator!=(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename Container>
[[nodiscard]] constexpr bool operator<(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return lhs.index() > rhs.index();
}

template <typename Container>
[[nodiscard]] constexpr bool operator>(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return lhs.index() < rhs.index();
}

template <typename Container>
[[nodiscard]] constexpr bool operator<=(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return !(lhs > rhs);
}

template <typename Container>
[[nodiscard]] constexpr bool operator>=(
    const sparse_set_iterator<Container> &lhs,
    const sparse_set_iterator<Container> &rhs) noexcept {
  return !(lhs < rhs);
}

}  // namespace internal

/**
 * \brief 散在して存在してるデータの集合を扱う
 * データの挿入、削除によってデータの順序については保証されない
 * \tparam Entity entity
 * \tparam Allocator allocator
 */
template <typename Entity, typename Allocator>
class basic_sparse_set {
  using alloc_traits = std::allocator_traits<Allocator>;
  static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>,
                "Invalid value type");
  using sparse_container_type =
      std::vector<typename alloc_traits::pointer,
                  typename alloc_traits::template rebind_alloc<
                      typename alloc_traits::pointer>>;
  using packed_container_type = std::vector<Entity, Allocator>;
  using EntityTraits = BecsTraits<Entity>;

  [[nodiscard]] static constexpr std::size_t fast_mod(
      const std::size_t value, const std::size_t mod) noexcept {
    return value & (mod - 1u);
  }

  [[nodiscard]] auto sparse_ptr(const Entity ent) const {
    const auto pos = static_cast<size_type>(traits_type::to_entity(ent));
    const auto page = pos / traits_type::page_size;
    return (page < sparse.size() && sparse[page])
               ? (sparse[page] + fast_mod(pos, traits_type::page_size))
               : nullptr;
  }

  [[nodiscard]] auto &sparse_ref(const Entity ent) const {
    const auto pos = static_cast<size_type>(traits_type::to_entity(ent));
    return sparse[pos / traits_type::page_size]
                 [fast_mod(pos, traits_type::page_size)];
  }

  [[nodiscard]] auto to_iterator(const Entity ent) const {
    return --(end() - index(ent));
  }

  [[nodiscard]] auto &assure_at_least(const Entity ent) {
    const auto pos = static_cast<size_type>(traits_type::to_entity(ent));
    const auto page = pos / traits_type::page_size;

    if (!(page < sparse.size())) {
      sparse.resize(page + 1u, nullptr);
    }

    if (!sparse[page]) {
      auto page_allocator{packed.get_allocator()};
      sparse[page] =
          alloc_traits::allocate(page_allocator, traits_type::page_size);
      std::uninitialized_fill(sparse[page],
                              sparse[page] + traits_type::page_size, null);
    }
    auto &elem = sparse[page][pos & (traits_type::page_size - 1)];
    return elem;
  }

  void release_sparse_pages() {
    auto page_allocator{packed.get_allocator()};

    for (auto &&page : sparse) {
      if (page != nullptr) {
        std::destroy(page, page + traits_type::page_size);
        alloc_traits::deallocate(page_allocator, page, traits_type::page_size);
        page = nullptr;
      }
    }
  }

 public:
  using basic_iterator = internal::sparse_set_iterator<packed_container_type>;

  /**
   * \brief SparseSet 内から Entity を削除する
   * \param first エンティティ範囲の先頭のイテレーター
   * \param last エンティティ範囲の末尾のイテレーター
   */
  virtual void pop(basic_iterator first, basic_iterator last) {
    if (mode == deletion_policy::swap_and_pop) {
      for (; first != last; ++first) {
        swap_and_pop(first);
      }
    } else {
      for (; first != last; ++first) {
        in_place_pop(first);
      }
    }
  }

  virtual void pop_all() {
    if (const auto prev = std::exchange(free_list, tombstone); prev == null) {
      for (auto first = begin(); !(first.index() < 0); ++first) {
        sparse_ref(*first) = null;
      }
    } else {
      for (auto first = begin(); !(first.index() < 0); ++first) {
        if (*first != tombstone) {
          sparse_ref(*first) = null;
        }
      }
    }

    packed.clear();
  }

  /**
   * \brief Entity を SparseSet に割り当てます。
   * \param ent 有効な識別子
   * \param force_back 逆側挿入.
   * \return Iterator 配置された要素.
   */
  virtual basic_iterator try_emplace(const Entity ent, const bool force_back,
                                     const void * = nullptr) {
    if (auto &elem = assure_at_least(ent); free_list == null || force_back) {
      packed.push_back(ent);
      elem = traits_type::combine(
          static_cast<typename traits_type::EntityType>(packed.size() - 1u),
          traits_type::to_integral(ent));
      return begin();
    } else {
      const auto pos =
          static_cast<size_type>(traits_type::to_entity(free_list));
      elem = traits_type::combine(traits_type::to_integral(free_list),
                                  traits_type::to_integral(ent));
      free_list = std::exchange(packed[pos], ent);
      return --(end() - pos);
    }
  }

  /**
   * \brief 指定の場所で２つの要素を入れ替える.
   */
  virtual void swap_at(const std::size_t lhs, const std::size_t rhs) {
    const auto entity = static_cast<typename traits_type::EntityType>(lhs);
    const auto other = static_cast<typename traits_type::EntityType>(rhs);

    sparse_ref(packed[lhs]) =
        traits_type::combine(other, traits_type::to_integral(packed[lhs]));
    sparse_ref(packed[rhs]) =
        traits_type::combine(entity, traits_type::to_integral(packed[rhs]));

    using std::swap;
    swap(packed[lhs], packed[rhs]);
  }

  /**
   * \brief SparseSet内からEntityを削除する
   * \param it 削除したい要素のイテレーター
   */
  void swap_and_pop(const basic_iterator it) {
    auto &self = sparse_ref(*it);
    const auto ent = traits_type::to_entity(self);
    sparse_ref(packed.back()) =
        traits_type::combine(ent, traits_type::to_integral(packed.back()));
    packed[static_cast<size_type>(ent)] = packed.back();
    // lazy self-assignment guard
    self = null;
    packed.pop_back();
  }

  /**
   * \brief SparseSet内からEntityを削除する
   * \param it 削除したい要素のイテレーター
   */
  void in_place_pop(const basic_iterator it) {
    const auto ent =
        traits_type::to_entity(std::exchange(sparse_ref(*it), null));
    packed[static_cast<size_type>(ent)] =
        std::exchange(free_list, traits_type::combine(ent, tombstone));
  }

 public:
  using traits_type = BecsTraits<Entity>;
  using entity_type = typename traits_type::value_type;
  using version_type = typename traits_type::VersionType;
  using size_type = std::size_t;
  using allocator_type = Allocator;
  using pointer = typename packed_container_type::const_pointer;
  using iterator = basic_iterator;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = reverse_iterator;
  /**
   * \brief 先頭要素のイテレータを返す
   * \return 先頭のイテレータ
   */
  [[nodiscard]] const_iterator begin() const noexcept {
    const auto pos =
        static_cast<typename iterator::difference_type>(packed.size());
    return iterator{packed, pos};
  }

  [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

  /**
   * \brief 末尾の次の要素のイテレータを返します。
   * \return 末尾の次の要素のイテレータ
   */
  [[nodiscard]] iterator end() const noexcept { return iterator{packed, {}}; }

  [[nodiscard]] const_iterator cend() const noexcept { return end(); }

  /**
   * \brief SparseSet内の要素数を返す
   * \return 要素数.
   */
  [[nodiscard]] size_type size() const noexcept { return packed.size(); }

  /**
   * \brief SparseSet内が空かどうかをチェックする
   * \return スパース セットが空の場合は true、そうでない場合は false
   */
  [[nodiscard]] bool empty() const noexcept { return packed.empty(); }

  /**
   * \brief 識別子に含まれるバージョンを返す
   * \param identifier 有効な識別子
   * \return 指定された識別子のバージョン (存在する場合)、廃棄 (tombstone)
   */
  [[nodiscard]] version_type current(
      const entity_type identifier) const noexcept {
    const auto elem = sparse_ptr(identifier);
    constexpr auto fallback = traits_type::to_version(tombstone);
    return elem ? traits_type::to_version(*elem) : fallback;
  }

  /**
   * \brief Entityの位置を取得する
   * \param elem 有効な識別子
   * \return SparseSetの Entity の位置
   */
  [[nodiscard]] size_type index(const entity_type elem) const noexcept {
    return static_cast<size_type>(traits_type::to_entity(sparse_ref(elem)));
  }

  /**
   * \brief Entityのバージョン(世代)を上げる.
   * \param elem 有効な識別子
   */
  void bump(const entity_type elem) {
    auto &entity = sparse_ref(elem);
    entity = traits_type::combine(traits_type::to_integral(entity),
                                  traits_type::to_integral(elem));
    packed[static_cast<size_type>(traits_type::to_entity(entity))] = elem;
  }

  /**
   * \brief SparseSet内の2つのEntityを入れ替える
   *
   * \param lhs 有効な識別子
   * \param rhs 有効な識別子
   */
  void swap_elements(const entity_type lhs, const entity_type rhs) {
    const auto from = index(lhs);
    const auto to = index(rhs);
    swap_or_move(from, to);
    swap_at(from, to);
  }

  // TODO compact
  /**
   * \brief 余分な Entity を削除する
   */
  void compact() {}

  template <typename Compare, typename Sort = std_sort, typename... Args>
  void sort_n(const size_type length, Compare compare, Sort algo = Sort{},
              Args &&...args) {
    algo(packed.rend() - length, packed.rend(), std::move(compare),
         std::forward<Args>(args)...);

    for (size_type pos{}; pos < length; ++pos) {
      auto curr = pos;
      auto next = index(packed[curr]);

      while (curr != next) {
        const auto idx = index(packed[next]);
        const auto entt = packed[curr];

        swap_at(next, idx);
        const auto entity =
            static_cast<typename EntityTraits::EntityType>(curr);
        sparse_ref(entt) = EntityTraits::combine(
            entity, EntityTraits::to_integral(packed[curr]));
        curr = std::exchange(next, idx);
      }
    }
  }

  template <typename Compare, typename Sort = std_sort, typename... Args>
  void sort(Compare compare, Sort algo = Sort{}, Args &&...args) {
    sort_n(packed.size(), std::move(compare), std::move(algo),
           std::forward<Args>(args)...);
  }

  /**
   * \brief 境界チェックを行い要素を取得する
   * \param pos 取得したい Entity の位置
   * \return 指定した場所に Entity がある場合はそのEntityを返す。なければ null
   */
  [[nodiscard]] entity_type at(const size_type pos) const noexcept {
    return pos < packed.size() ? packed[pos] : null;
  }

  /**
   * \brief 要素を取得する
   * \param pos 取得したい Entity の位置
   * \return 指定した場所にある Entity
   */
  [[nodiscard]] entity_type operator[](const size_type pos) const noexcept {
    return packed[pos];
  }

  /**
   * \brief 戻り値の型
   * \return
   */
  const type_info &type() const noexcept { return *info; }

  /*! @brief 変数を派生クラスに転送します */
  virtual void bind(std::any) noexcept {}

  /**
   * \brief 存在するかSparseSet内に存在するかをチェックする
   * \param elem 有効な識別子
   * \return 存在すれば true、存在しなければ false
   */
  [[nodiscard]] bool contains(const entity_type elem) const noexcept {
    const auto elem_p = sparse_ptr(elem);
    constexpr auto cap = traits_type::to_entity(null);
    return elem_p && (((~cap & traits_type::to_integral(elem)) ^
                       traits_type::to_integral(*elem_p)) < cap);
  }

  /**
   * \brief Entity を検索する。
   * \param elem 有効な識別子
   * \return Entity
   * が存在すればそのイテレータを返す。存在しなければ末尾のイテレータを返す。
   */
  [[nodiscard]] iterator find(const entity_type elem) const noexcept {
    return contains(elem) ? to_iterator(elem) : end();
  }

  void erase(const entity_type elem) {
    const auto it = to_iterator(elem);
    pop(it, it + 1u);
  }
  bool remove(const entity_type elem) {
    return contains(elem) && (erase(elem), true);
  }
  explicit basic_sparse_set(const type_info &elem,
                            deletion_policy pol = deletion_policy::swap_and_pop,
                            const allocator_type &allocator = {})
      : sparse{allocator},
        packed{allocator},
        info{&elem},
        free_list{tombstone},
        mode{pol} {}

 private:
  virtual void swap_or_move(const std::size_t, const std::size_t) {}
  virtual void move_element(const std::size_t, const std::size_t) {}

 private:
  sparse_container_type sparse;
  packed_container_type packed;
  const type_info *info;
  entity_type free_list;
  deletion_policy mode;
};
}  // namespace becs

// @Storage.h
// @brief
// @author ICE
// @date 2023/03/18
//
// @details

#pragma once
#include "BecsComponent.h"
#include "CompressedPair.h"
#include "EntityFwd.h"
#include "SparseSet.h"
#include "TypeInfo.h"
#include "TypeTraits.h"
namespace becs {

namespace internal {

template <typename Container, typename Size>
class storage_iterator final {
  friend storage_iterator<const Container, Size>;

  using container_type = std::remove_const_t<Container>;
  using alloc_traits =
      std::allocator_traits<typename container_type::allocator_type>;

  using iterator_traits = std::iterator_traits<std::conditional_t<
      std::is_const_v<Container>,
      typename alloc_traits::template rebind_traits<
          typename std::pointer_traits<typename container_type::value_type>::
              element_type>::const_pointer,
      typename alloc_traits::template rebind_traits<
          typename std::pointer_traits<
              typename container_type::value_type>::element_type>::pointer>>;

 public:
  using value_type = typename iterator_traits::value_type;
  using pointer = typename iterator_traits::pointer;
  using reference = typename iterator_traits::reference;
  using difference_type = typename iterator_traits::difference_type;
  using iterator_category = std::random_access_iterator_tag;

  constexpr storage_iterator() noexcept = default;

  constexpr storage_iterator(Container *ref, const difference_type idx) noexcept
      : payload{ref}, offset{idx} {}

  template <bool Const = std::is_const_v<Container>,
            typename = std::enable_if_t<Const>>
  constexpr storage_iterator(
      const storage_iterator<std::remove_const_t<Container>, Size>
          &other) noexcept
      : storage_iterator{other.payload, other.offset} {}

  constexpr storage_iterator &operator++() noexcept { return --offset, *this; }

  constexpr storage_iterator operator++(int) noexcept {
    storage_iterator orig = *this;
    return ++(*this), orig;
  }

  constexpr storage_iterator &operator--() noexcept { return ++offset, *this; }

  constexpr storage_iterator operator--(int) noexcept {
    storage_iterator orig = *this;
    return operator--(), orig;
  }

  constexpr storage_iterator &operator+=(const difference_type value) noexcept {
    offset -= value;
    return *this;
  }

  constexpr storage_iterator operator+(
      const difference_type value) const noexcept {
    storage_iterator copy = *this;
    return (copy += value);
  }

  constexpr storage_iterator &operator-=(const difference_type value) noexcept {
    return (*this += -value);
  }

  constexpr storage_iterator operator-(
      const difference_type value) const noexcept {
    return (*this + -value);
  }

  [[nodiscard]] constexpr reference operator[](
      const difference_type value) const noexcept {
    const auto pos = index() - value;
    return (*payload)[pos / Size::value][fast_mod(pos, Size::value)];
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
    const auto pos = index();
    return (*payload)[pos / Size::value] + fast_mod(pos, Size::value);
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return *operator->();
  }

  [[nodiscard]] constexpr difference_type index() const noexcept {
    return offset - 1;
  }

 private:
  Container *payload;
  difference_type offset;
};

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr std::ptrdiff_t operator-(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return rhs.index() - lhs.index();
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator==(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return lhs.index() == rhs.index();
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator!=(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return !(lhs == rhs);
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator<(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return lhs.index() > rhs.index();
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator>(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return lhs.index() < rhs.index();
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator<=(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return !(lhs > rhs);
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator>=(
    const storage_iterator<Lhs...> &lhs,
    const storage_iterator<Rhs...> &rhs) noexcept {
  return !(lhs < rhs);
}

template <typename It, typename... Other>
class extended_storage_iterator final {
  template <typename Iter, typename... Args>
  friend class extended_storage_iterator;

 public:
  using iterator_type = It;
  using difference_type = std::ptrdiff_t;
  using value_type = decltype(std::tuple_cat(
      std::make_tuple(*std::declval<It>()),
      std::forward_as_tuple(*std::declval<Other>()...)));
  using pointer = input_iterator_pointer<value_type>;
  using reference = value_type;
  using iterator_category = std::input_iterator_tag;

  constexpr extended_storage_iterator() : it{} {}

  constexpr extended_storage_iterator(It base, Other... other)
      : it{base, other...} {}

  template <typename... Args,
            typename =
                std::enable_if_t<(!std::is_same_v<Other, Args> && ...) &&
                                 (std::is_constructible_v<Other, Args> && ...)>>
  constexpr extended_storage_iterator(
      const extended_storage_iterator<It, Args...> &other)
      : it{other.it} {}

  constexpr extended_storage_iterator &operator++() noexcept {
    return ++std::get<It>(it), (++std::get<Other>(it), ...), *this;
  }

  constexpr extended_storage_iterator operator++(int) noexcept {
    extended_storage_iterator orig = *this;
    return ++(*this), orig;
  }

  [[nodiscard]] constexpr pointer operator->() const noexcept {
    return operator*();
  }

  [[nodiscard]] constexpr reference operator*() const noexcept {
    return {*std::get<It>(it), *std::get<Other>(it)...};
  }

  [[nodiscard]] constexpr iterator_type base() const noexcept {
    return std::get<It>(it);
  }

  template <typename... Lhs, typename... Rhs>
  friend constexpr bool operator==(
      const extended_storage_iterator<Lhs...> &,
      const extended_storage_iterator<Rhs...> &) noexcept;

 private:
  std::tuple<It, Other...> it;
};

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator==(
    const extended_storage_iterator<Lhs...> &lhs,
    const extended_storage_iterator<Rhs...> &rhs) noexcept {
  return std::get<0>(lhs.it) == std::get<0>(rhs.it);
}

template <typename... Lhs, typename... Rhs>
[[nodiscard]] constexpr bool operator!=(
    const extended_storage_iterator<Lhs...> &lhs,
    const extended_storage_iterator<Rhs...> &rhs) noexcept {
  return !(lhs == rhs);
}

}  // namespace internal

template <typename Type, typename Entity, typename Allocator, typename>
class basic_storage
    : public basic_sparse_set<Entity,
                              typename std::allocator_traits<
                                  Allocator>::template rebind_alloc<Entity>> {
  using alloc_traits = std::allocator_traits<Allocator>;
  static_assert(std::is_same_v<typename alloc_traits::value_type, Type>,
                "Invalid value type");
  using container_type =
      std::vector<typename alloc_traits::pointer,
                  typename alloc_traits::template rebind_alloc<
                      typename alloc_traits::pointer>>;
  using underlying_type =
      basic_sparse_set<Entity,
                       typename alloc_traits::template rebind_alloc<Entity>>;
  using underlying_iterator = typename underlying_type::basic_iterator;

  static constexpr bool is_pinned_type_v =
      !(std::is_move_constructible_v<Type> && std::is_move_assignable_v<Type>);

  [[nodiscard]] auto &element_at(const std::size_t pos) const {
    return payload[pos / traits_type::page_size]
                  [fast_mod(pos, traits_type::page_size)];
  }

  [[nodiscard]] static inline constexpr std::size_t fast_mod(
      const std::size_t value, const std::size_t mod) noexcept {
    return value & (mod - 1u);
  }
  auto assure_at_least(const std::size_t pos) {
    const auto idx = pos / traits_type::page_size;

    if (!(idx < payload.size())) {
      auto curr = payload.size();
      allocator_type page_allocator{get_allocator()};
      payload.resize(idx + 1u, nullptr);

      try {
        for (const auto last = payload.size(); curr < last; ++curr) {
          payload[curr] =
              alloc_traits::allocate(page_allocator, traits_type::page_size);
        }
      } catch (...) {
        payload.resize(curr);
        throw;
      }
    }

    return payload[idx] + fast_mod(pos, traits_type::page_size);
  }

  template <typename... Args>
  auto emplace_element(const Entity elem, const bool force_back,
                       Args &&...args) {
    const auto it = base_type::try_emplace(elem, force_back);

    try {
      auto val = assure_at_least(static_cast<size_type>(it.index()));
      std::uninitialized_construct_using_allocator(
          std::to_address(val), packed.second(), std::forward<Args>(args)...);

    } catch (...) {
      base_type::pop(it, it + 1u);
      throw;
    }

    return it;
  }

  void shrink_to_size(const std::size_t sz) {
    for (auto pos = sz, length = base_type::size(); pos < length; ++pos) {
      if constexpr (traits_type::in_place_delete) {
        if (base_type::at(pos) != tombstone) {
          std::destroy_at(std::addressof(element_at(pos)));
        }
      } else {
        std::destroy_at(std::addressof(element_at(pos)));
      }
    }

    allocator_type page_allocator{get_allocator()};
    const auto from =
        (sz + traits_type::page_size - 1u) / traits_type::page_size;

    for (auto pos = from, last = payload.size(); pos < last; ++pos) {
      alloc_traits::deallocate(page_allocator, payload[pos],
                               traits_type::page_size);
    }

    payload.resize(from);
  }

  void swap_at(const std::size_t lhs, const std::size_t rhs) final {
    using std::swap;
    swap(element_at(lhs), element_at(rhs));
  }

  void copy(Entity src_entity, std::shared_ptr<basic_sparse_set<>> &dst,
            Entity dst_entity) const override {
    if (!dst) {
      dst = std::make_shared<basic_storage<Type>>();
    }
    if (base_type::contains(src_entity)) {
      dst->try_emplace(dst_entity, false, try_get(src_entity));
    }
  }

 private:
  void swap_or_move([[maybe_unused]] const std::size_t from,
                    [[maybe_unused]] const std::size_t to) override {}
  void move_element(const std::size_t from, const std::size_t to) final {
    auto &elem = element_at(from);
    std::uninitialized_construct_using_allocator(
        std::to_address(assure_at_least(to)), packed.second(), std::move(elem));
    std::destroy_at(std::addressof(elem));
  }

 protected:
  void pop(underlying_iterator first, underlying_iterator last) override {
    for (; first != last; ++first) {
      const auto &elem = *first;
      const auto index = base_type::index(elem);
      auto &val = element_at(index);

      if constexpr (traits_type::in_place_delete) {
        base_type::in_place_pop(first);
        std::destroy_at(std::addressof(val));
      } else {
        auto &other = element_at(base_type::size() - 1u);
        [[maybe_unused]] auto unused = std::exchange(val, std::move(other));
        std::destroy_at(std::addressof(other));
        base_type::swap_and_pop(first);
      }
    }
  }

  /**
   * \brief 全ての Entity を削除する
   */
  void pop_all() override {
    for (auto first = base_type::begin(); !(first.index() < 0); ++first) {
      if constexpr (traits_type::in_place_delete) {
        if (*first != tombstone) {
          base_type::in_place_pop(first);
          std::destroy_at(std::addressof(
              element_at(static_cast<size_type>(first.index()))));
        }
      } else {
        base_type::swap_and_pop(first);
        std::destroy_at(
            std::addressof(element_at(static_cast<size_type>(first.index()))));
      }
    }
  }

  underlying_iterator try_emplace([[maybe_unused]] const Entity elem,
                                  [[maybe_unused]] const bool force_back,
                                  const void *value) override {
    if (value) {
      if constexpr (std::is_copy_constructible_v<value_type>) {
        return emplace_element(elem, force_back,
                               *static_cast<const value_type *>(value));
      } else {
        return base_type::end();
      }
    } else {
      if constexpr (std::is_default_constructible_v<value_type>) {
        return emplace_element(elem, force_back);
      } else {
        return base_type::end();
      }
    }
  }

 public:
  using base_type = underlying_type;
  using value_type = Type;
  using traits_type = component_traits<value_type>;
  using entity_type = Entity;
  using size_type = std::size_t;
  using allocator_type = Allocator;
  using pointer = typename container_type::pointer;
  using const_pointer = typename alloc_traits::template rebind_traits<
      typename alloc_traits::const_pointer>::const_pointer;
  using iterator = internal::storage_iterator<
      container_type,
      std::integral_constant<size_type, traits_type::page_size>>;
  using const_iterator = internal::storage_iterator<
      const container_type,
      std::integral_constant<size_type, traits_type::page_size>>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using iterable = iterable_adaptor<internal::extended_storage_iterator<
      typename base_type::iterator, iterator>>;
  using const_iterable = iterable_adaptor<internal::extended_storage_iterator<
      typename base_type::const_iterator, const_iterator>>;

  basic_storage() : basic_storage{allocator_type{}} {}

  explicit basic_storage(const allocator_type &allocator)
      : base_type{type_id<value_type>(),
                  deletion_policy{traits_type::in_place_delete}, allocator},
        payload{allocator} {}

  basic_storage(basic_storage &&other) noexcept
      : base_type{std::move(other)}, payload{std::move(other.payload)} {}
  basic_storage(basic_storage &&other, const allocator_type &allocator) noexcept
      : base_type{std::move(other), allocator},
        payload{std::move(other.payload), allocator} {}
  ~basic_storage() {}
  void swap(basic_storage &other) {
    using std::swap;
    base_type::swap(other);
    swap(payload, other.payload);
  }
  [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
    return payload.get_allocator();
  }

  [[nodiscard]] const_pointer raw() const noexcept { return payload.data(); }

  [[nodiscard]] pointer raw() noexcept { return payload.data(); }

  [[nodiscard]] const_iterator cbegin() const noexcept {
    const auto pos =
        static_cast<typename iterator::difference_type>(base_type::size());
    return const_iterator{&payload, pos};
  }

  [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

  [[nodiscard]] iterator begin() noexcept {
    const auto pos =
        static_cast<typename iterator::difference_type>(base_type::size());
    return iterator{&payload, pos};
  }

  [[nodiscard]] const_iterator cend() const noexcept {
    return const_iterator{&payload, {}};
  }

  [[nodiscard]] const_iterator end() const noexcept { return cend(); }

  [[nodiscard]] iterator end() noexcept { return iterator{&payload, {}}; }

  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return std::make_reverse_iterator(cend());
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return crbegin();
  }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }

  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept { return crend(); }

  [[nodiscard]] reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }
  [[nodiscard]] const value_type &get(const entity_type elem) const noexcept {
    return element_at(base_type::index(elem));
  }

  [[nodiscard]] value_type &get(const entity_type elem) noexcept {
    return const_cast<value_type &>(std::as_const(*this).get(elem));
  }
  [[nodiscard]] const void *try_get(
      const entity_type elem) const noexcept override {
    if (!base_type::contains(elem)) return nullptr;
    return static_cast<void *>(
        std::addressof(element_at(base_type::index(elem))));
  }

  [[nodiscard]] void *try_get(const entity_type elem) noexcept override {
    return const_cast<void *>(std::as_const(*this).try_get(elem));
  }

  [[nodiscard]] std::tuple<const value_type &> get_as_tuple(
      const entity_type elem) const noexcept {
    return std::forward_as_tuple(get(elem));
  }
  [[nodiscard]] std::tuple<value_type &> get_as_tuple(
      const entity_type elem) noexcept {
    return std::forward_as_tuple(get(elem));
  }
  template <typename... Args>
  value_type &emplace(const entity_type elem, Args &&...args) {
    if constexpr (std::is_aggregate_v<value_type>) {
      const auto it =
          emplace_element(elem, false, Type{std::forward<Args>(args)...});
      return element_at(static_cast<size_type>(it.index()));
    } else {
      const auto it = emplace_element(elem, false, std::forward<Args>(args)...);
      return element_at(static_cast<size_type>(it.index()));
    }
  }

  template <typename... Func>
  value_type &patch(const entity_type elem, Func &&...func) {
    const auto idx = base_type::index(elem);
    auto &val = element_at(idx);
    (std::forward<Func>(func)(val), ...);
    return val;
  }

  template <typename It>
  void insert(It first, It last, const value_type &value = {}) {
    for (; first != last; ++first) {
      emplace_element(*first, true, value);
    }
  }

  template <typename EIt, typename CIt,
            typename = std::enable_if_t<std::is_same_v<
                typename std::iterator_traits<CIt>::value_type, value_type>>>
  void insert(EIt first, EIt last, CIt from) {
    for (; first != last; ++first, ++from) {
      emplace_element(*first, true, *from);
    }
  }

  [[nodiscard]] iterable each() noexcept {
    return {internal::extended_storage_iterator{base_type::begin(), begin()},
            internal::extended_storage_iterator{base_type::end(), end()}};
  }

  [[nodiscard]] const_iterable each() const noexcept {
    return {internal::extended_storage_iterator{base_type::cbegin(), cbegin()},
            internal::extended_storage_iterator{base_type::cend(), cend()}};
  }

 private:
  compressed_pair<container_type, Allocator> packed;

  container_type payload;
};

template <typename Entity, typename Allocator>
class basic_storage<Entity, Entity, Allocator>
    : public basic_sparse_set<Entity, Allocator> {
  using alloc_traits = std::allocator_traits<Allocator>;
  static_assert(std::is_same_v<typename alloc_traits::value_type, Entity>,
                "Invalid value type");
  using underlying_type =
      basic_sparse_set<Entity,
                       typename alloc_traits::template rebind_alloc<Entity>>;
  using underlying_iterator = typename underlying_type::basic_iterator;
  using local_traits_type = BecsTraits<Entity>;

  [[nodiscard]] auto entity_at(const std::size_t pos) const noexcept {
    const auto entity = local_traits_type::combine(
        static_cast<typename local_traits_type::EntityType>(pos), {});
    return entity;
  }

 private:
 protected:
  /**
   * @brief storage から Entity を削除します
   * @param first 先頭要素のイテレーター
   * @param last 末尾要素のイテレーター
   */
  void pop(underlying_iterator first, underlying_iterator last) override {
    for (; first != last; ++first) {
      if (const auto pos = base_type::index(*first); pos < length) {
        base_type::bump(local_traits_type::next(*first));

        if (pos != --length) {
          base_type::swap_at(pos, length);
        }
      }
    }
  }

  /**
   * \brief 全てのEntityを削除する
   */
  void pop_all() override {
    length = 0u;
    base_type::pop_all();
  }

  underlying_iterator try_emplace(const Entity hint, const bool,
                                  const void *) override {
    return base_type::find(spawn(hint));
  }

 public:
  using base_type = basic_sparse_set<Entity, Allocator>;
  using value_type = Entity;
  using traits_type = component_traits<void>;
  using entity_type = Entity;
  using size_type = std::size_t;
  using allocator_type = Allocator;
  using iterable = iterable_adaptor<
      internal::extended_storage_iterator<typename base_type::iterator>>;
  using const_iterable = iterable_adaptor<
      internal::extended_storage_iterator<typename base_type::const_iterator>>;

  basic_storage() : basic_storage{allocator_type{}} {}

  explicit basic_storage(const allocator_type &allocator)
      : base_type{type_id<value_type>(), deletion_policy::swap_and_pop,
                  allocator},
        length{} {}
  basic_storage(basic_storage &&other) noexcept
      : base_type{std::move(other)},
        length{std::exchange(other.length, size_type{})} {}

  basic_storage(basic_storage &&other, const allocator_type &allocator) noexcept
      : base_type{std::move(other), allocator},
        length{std::exchange(other.length, size_type{})} {}

  basic_storage &operator=(basic_storage &&other) noexcept {
    base_type::operator=(std::move(other));
    length = std::exchange(other.length, size_type{});
    return *this;
  }

  [[nodiscard]] std::tuple<> get_as_tuple(
      [[maybe_unused]] const entity_type elem) const noexcept {
    return std::tuple{};
  }

  void swap(basic_storage &other) {
    using std::swap;
    base_type::swap(other);
    swap(length, other.length);
  }

  /**
   * \brief 新しく生成するか、破棄済みのものをリサイクルします。
   * \return 有効な識別子
   */
  entity_type spawn() {
    if (length == base_type::size()) {
      return *base_type::try_emplace(entity_at(length++), true);
    }

    return base_type::operator[](length++);
  }

  /*
   * \brief 新しく生成するか、破棄済みのものをリサイクルします。
   * \param hint ヒントとして使う識別子
   * \return 有効な識別子
   */
  entity_type spawn(const entity_type hint) {
    if (hint == null || hint == tombstone) {
      return spawn();
    } else if (const auto curr = local_traits_type::construct(
                   local_traits_type::to_entity(hint),
                   base_type::current(hint));
               curr == tombstone) {
      const auto pos =
          static_cast<size_type>(local_traits_type::to_entity(hint));

      while (!(pos < base_type::size())) {
        base_type::try_emplace(entity_at(base_type::size()), true);
      }

      base_type::swap_at(pos, length++);
    } else if (const auto idx = base_type::index(curr); idx < length) {
      return spawn();
    } else {
      base_type::swap_at(idx, length++);
    }

    base_type::bump(hint);

    return hint;
  }

  /**
   * @brief 範囲内の各要素に識別子を割り当てます
   * @tparam It 可変前方向イテレーター型
   * @param first 生成範囲の先頭イテレーター
   * @param last 生成範囲の末尾イテレーター
   */
  template <typename It>
  void spawn(It first, It last) {
    for (const auto sz = base_type::size(); first != last && length != sz;
         ++first, ++length) {
      *first = base_type::operator[](length);
    }

    for (; first != last; ++first) {
      *first = *base_type::try_emplace(entity_at(length++), true);
    }
  }

  template <typename It>
  size_type pack(It first, It last) {
    size_type len = length;

    for (; first != last; ++first, --len) {
      const auto pos = base_type::index(*first);
      base_type::swap_at(pos, static_cast<size_type>(len - 1u));
    }

    return (length - len);
  }

  [[nodiscard]] size_type in_use() const noexcept { return length; }

  void in_use(const size_type len) noexcept { length = len; }

  [[nodiscard]] iterable each() noexcept {
    return {internal::extended_storage_iterator{base_type::end() - length},
            internal::extended_storage_iterator{base_type::end()}};
  }

  [[nodiscard]] const_iterable each() const noexcept {
    return {internal::extended_storage_iterator{base_type::cend() - length},
            internal::extended_storage_iterator{base_type::cend()}};
  }

 private:
  size_type length;
};
}  // namespace becs

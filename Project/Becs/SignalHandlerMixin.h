// @SignalHandlerMixin.h
// @brief
// @author ICE
// @date 2023/03/18
//
// @details

#pragma once
#include <any>

#include "EntityFwd.h"
#include "SignalHandle.h"
namespace becs {
template <typename Type>
class sigh_mixin final : public Type {
  using basic_registry_type =
      BasicRegistry<typename Type::entity_type,
                    typename Type::base_type::allocator_type>;
  using sigh_type = SignalHandler<void(basic_registry_type &,
                                       const typename Type::entity_type),
                                  typename Type::allocator_type>;
  using underlying_iterator = typename Type::base_type::basic_iterator;

  void pop(underlying_iterator first, underlying_iterator last) final {
    if (destruction.empty()) {
      Type::pop(first, last);
    } else {
      for (; first != last; ++first) {
        const auto entt = *first;
        destruction.publish(*owner, entt);
        const auto it = Type::find(entt);
        Type::pop(it, it + 1u);
      }
    }
  }

  void pop_all() final {
    if (!destruction.empty()) {
      for (auto pos = Type::each().begin().base().index(); !(pos < 0); --pos) {
        if constexpr (Type::traits_type::in_place_delete) {
          if (const auto entt =
                  Type::operator[](static_cast<typename Type::size_type>(pos));
              entt != tombstone) {
            destruction.publish(*owner, entt);
          }
        } else {
          destruction.publish(
              *owner,
              Type::operator[](static_cast<typename Type::size_type>(pos)));
        }
      }
    }

    Type::pop_all();
  }

  underlying_iterator try_emplace(const typename Type::entity_type entt,
                                  const bool force_back,
                                  const void *value) final {
    const auto it = Type::try_emplace(entt, force_back, value);

    if (it != Type::base_type::end()) {
      construction.publish(*owner, *it);
    }

    return it;
  }

 public:
  using allocator_type = typename Type::allocator_type;
  using entity_type = typename Type::entity_type;
  using registry_type = basic_registry_type;

  sigh_mixin() : sigh_mixin{allocator_type{}} {}

  explicit sigh_mixin(const allocator_type &allocator)
      : Type{allocator},
        owner{},
        construction{allocator},
        destruction{allocator},
        update{allocator} {}

  sigh_mixin(sigh_mixin &&other) noexcept
      : Type{std::move(other)},
        owner{other.owner},
        construction{std::move(other.construction)},
        destruction{std::move(other.destruction)},
        update{std::move(other.update)} {}

  sigh_mixin(sigh_mixin &&other, const allocator_type &allocator) noexcept
      : Type{std::move(other), allocator},
        owner{other.owner},
        construction{std::move(other.construction), allocator},
        destruction{std::move(other.destruction), allocator},
        update{std::move(other.update), allocator} {}

  sigh_mixin &operator=(sigh_mixin &&other) noexcept {
    Type::operator=(std::move(other));
    owner = other.owner;
    construction = std::move(other.construction);
    destruction = std::move(other.destruction);
    update = std::move(other.update);
    return *this;
  }

  void swap(sigh_mixin &other) {
    using std::swap;
    Type::swap(other);
    swap(owner, other.owner);
    swap(construction, other.construction);
    swap(destruction, other.destruction);
    swap(update, other.update);
  }

  [[nodiscard]] auto on_construct() noexcept { return sink{construction}; }
  [[nodiscard]] auto on_update() noexcept { return sink{update}; }
  [[nodiscard]] auto on_destroy() noexcept { return sink{destruction}; }

  template <typename... Args>
  decltype(auto) emplace(const entity_type entt, Args &&...args) {
    Type::emplace(entt, std::forward<Args>(args)...);
    construction.publish(*owner, entt);
    return this->get(entt);
  }

  template <typename... Func>
  decltype(auto) patch(const entity_type entt, Func &&...func) {
    Type::patch(entt, std::forward<Func>(func)...);
    update.publish(*owner, entt);
    return this->get(entt);
  }

  template <typename It, typename... Args>
  void insert(It first, It last, Args &&...args) {
    Type::insert(first, last, std::forward<Args>(args)...);

    if (!construction.empty()) {
      for (; first != last; ++first) {
        construction.publish(*owner, *first);
      }
    }
  }

  /**
   * @brief 識別子を生成する、もしくは破棄済みのものをリサイクルします。
   * @return 有効な識別子
   */
  entity_type spawn() {
    const auto entt = Type::spawn();
    construction.publish(*owner, entt);
    return entt;
  }

  entity_type spawn(const entity_type hint) {
    const auto entt = Type::spawn(hint);
    construction.publish(*owner, entt);
    return entt;
  }

  template <typename It>
  void spawn(It first, It last) {
    Type::spawn(first, last);

    if (!construction.empty()) {
      for (; first != last; ++first) {
        construction.publish(*owner, *first);
      }
    }
  }

  /**
   * @brief 派生クラスに登録する
   * @param ラップされた変数
   */
  void bind(std::any value) noexcept final {
    auto *reg = std::any_cast<basic_registry_type>(&value);
    owner = reg ? reg : owner;
    Type::bind(std::move(value));
  }

 private:
  basic_registry_type *owner;
  sigh_type construction;
  sigh_type destruction;
  sigh_type update;
};
}  // namespace becs
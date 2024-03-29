﻿// @SignalHandlerMixin.h
// @brief
// @author ICE
// @date 2023/03/18
//
// @details

#pragma once
#include <any>

#include "SignalHandle.h"
#include "EntityFwd.h"
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
  /*! @brief Allocator type. */
  using allocator_type = typename Type::allocator_type;
  /*! @brief Underlying entity identifier. */
  using entity_type = typename Type::entity_type;
  /*! @brief Expected registry type. */
  using registry_type = basic_registry_type;

  /*! @brief Default constructor. */
  sigh_mixin() : sigh_mixin{allocator_type{}} {}

  /**
   * @brief Constructs an empty storage with a given allocator.
   * @param allocator The allocator to use.
   */
  explicit sigh_mixin(const allocator_type &allocator)
      : Type{allocator},
        owner{},
        construction{allocator},
        destruction{allocator},
        update{allocator} {}

  /**
   * @brief Move constructor.
   * @param other The instance to move from.
   */
  sigh_mixin(sigh_mixin &&other) noexcept
      : Type{std::move(other)},
        owner{other.owner},
        construction{std::move(other.construction)},
        destruction{std::move(other.destruction)},
        update{std::move(other.update)} {}

  /**
   * @brief Allocator-extended move constructor.
   * @param other The instance to move from.
   * @param allocator The allocator to use.
   */
  sigh_mixin(sigh_mixin &&other, const allocator_type &allocator) noexcept
      : Type{std::move(other), allocator},
        owner{other.owner},
        construction{std::move(other.construction), allocator},
        destruction{std::move(other.destruction), allocator},
        update{std::move(other.update), allocator} {}

  /**
   * @brief Move assignment operator.
   * @param other The instance to move from.
   * @return This storage.
   */
  sigh_mixin &operator=(sigh_mixin &&other) noexcept {
    Type::operator=(std::move(other));
    owner = other.owner;
    construction = std::move(other.construction);
    destruction = std::move(other.destruction);
    update = std::move(other.update);
    return *this;
  }

  /**
   * @brief Exchanges the contents with those of a given storage.
   * @param other Storage to exchange the content with.
   */
  void swap(sigh_mixin &other) {
    using std::swap;
    Type::swap(other);
    swap(owner, other.owner);
    swap(construction, other.construction);
    swap(destruction, other.destruction);
    swap(update, other.update);
  }

  /**
   * @brief Returns a sink object.
   *
   * The sink returned by this function can be used to receive notifications
   * whenever a new instance is created and assigned to an Entity.<br/>
   * Listeners are invoked after the object has been assigned to the Entity.
   *
   * @sa sink
   *
   * @return A temporary sink object.
   */
  [[nodiscard]] auto on_construct() noexcept { return sink{construction}; }

  /**
   * @brief Returns a sink object.
   *
   * The sink returned by this function can be used to receive notifications
   * whenever an instance is explicitly updated.<br/>
   * Listeners are invoked after the object has been updated.
   *
   * @sa sink
   *
   * @return A temporary sink object.
   */
  [[nodiscard]] auto on_update() noexcept { return sink{update}; }

  /**
   * @brief Returns a sink object.
   *
   * The sink returned by this function can be used to receive notifications
   * whenever an instance is removed from an Entity and thus destroyed.<br/>
   * Listeners are invoked before the object has been removed from the Entity.
   *
   * @sa sink
   *
   * @return A temporary sink object.
   */
  [[nodiscard]] auto on_destroy() noexcept { return sink{destruction}; }

  /**
   * @brief Assigns entities to a storage.
   * @tparam Args Types of arguments to use to construct the object.
   * @param entt A valid identifier.
   * @param args Parameters to use to initialize the object.
   * @return A reference to the newly created object.
   */
  template <typename... Args>
  decltype(auto) emplace(const entity_type entt, Args &&...args) {
    Type::emplace(entt, std::forward<Args>(args)...);
    construction.publish(*owner, entt);
    return this->get(entt);
  }

  /**
   * @brief Patches the given instance for an entity.
   * @tparam Func Types of the function objects to invoke.
   * @param entt A valid identifier.
   * @param func Valid function objects.
   * @return A reference to the patched instance.
   */
  template <typename... Func>
  decltype(auto) patch(const entity_type entt, Func &&...func) {
    Type::patch(entt, std::forward<Func>(func)...);
    update.publish(*owner, entt);
    return this->get(entt);
  }

  /**
   * @brief Assigns entities to a storage.
   * @tparam It Type of input iterator.
   * @tparam Args Types of arguments to use to construct the objects assigned
   * to the entities.
   * @param first An iterator to the first element of the range of entities.
   * @param last An iterator past the last element of the range of entities.
   * @param args Parameters to use to initialize the objects assigned to the
   * entities.
   */
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

  /**
   * @brief Creates a new identifier or recycles a destroyed one.
   *
   * If the requested identifier isn't in use, the suggested one is used.
   * Otherwise, a new identifier is returned.
   *
   * @param hint Required identifier.
   * @return A valid identifier.
   */
  entity_type spawn(const entity_type hint) {
    const auto entt = Type::spawn(hint);
    construction.publish(*owner, entt);
    return entt;
  }

  /**
   * @brief Assigns each element in a range an identifier.
   * @tparam It Type of mutable forward iterator.
   * @param first An iterator to the first element of the range to generate.
   * @param last An iterator past the last element of the range to generate.
   */
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
   * @brief Forwards variables to derived classes, if any.
   * @param value A variable wrapped in an opaque container.
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
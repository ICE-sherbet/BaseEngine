// @Entity.h
// @brief コンポーネントにアクセスするためのハンドル
// @author ICE
// @date 2023/03/02
//
// @details

#pragma once
#include <cstdint>

#include "config.h"
#include "EntityFwd.h"
#include "Traits/Traits.h"

namespace becs {


namespace {

constexpr auto to_integral(const Entity id) noexcept {
  return static_cast<std::underlying_type_t<Entity>>(id);
}

class Null {
  template <typename Entity>
  using TraitsType = internal::BecsTraits<std::underlying_type_t<Entity>>;

public:
  template <typename Entity>
  explicit constexpr operator Entity() const noexcept {
    return Entity{TraitsType<Entity>::entity_mask};
  }

  constexpr bool operator==(Null) const noexcept { return true; }

  constexpr bool operator!=(Null) const noexcept { return false; }

  template <typename Entity>
  constexpr bool operator==(const Entity entity) const noexcept {
    return (to_integral(entity) & TraitsType<Entity>::entity_mask) ==
           to_integral(static_cast<Entity>(*this));
  }

  template <typename Entity>
  constexpr bool operator!=(const Entity entity) const noexcept {
    return !(entity == *this);
  }
};
}

inline constexpr Entity kNull = static_cast<Entity>(Null{});

template <typename Entity>
constexpr bool operator==(const Entity entity, Null other) noexcept {
  return other.operator==(entity);
}

template <typename Entity>
constexpr bool operator!=(const Entity entity, Null other) noexcept {
  return !(other == entity);
}


}  // namespace becs

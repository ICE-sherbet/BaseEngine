// @EntityFwd.h
// @brief
// @author ICE
// @date 2023/03/09
//
// @details

#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>

#include "TypeTraits.h"
#include "config.h"

namespace becs {

enum class Entity : IdType {};

/*! \brief ストレージ削除ポリシー. */
enum class deletion_policy : std::uint8_t { swap_and_pop = 0u, in_place = 1u };

template <typename Entity = Entity, typename = std::allocator<Entity>>
class basic_sparse_set;

template <typename Type, typename = Entity, typename = std::allocator<Type>,
          typename = void>
class basic_storage;

template <typename Type>
class sigh_mixin;

template <typename Type, typename Entity = Entity,
          typename Allocator = std::allocator<Type>, typename = void>
struct storage_type {
  using type = sigh_mixin<basic_storage<Type, Entity, Allocator>>;
};

/**
 * \brief ヘルパー型
 */
template <typename... Args>
using storage_type_t = typename storage_type<Args...>::type;

/**
 * \breif constness を保持する型からストレージへの変換ユーティリティ.
 * \tparam Type ストレージ値の型
 * \tparam Entity 有効な識別子
 */
template <typename Type, typename Entity = Entity,
          typename Allocator = std::allocator<std::remove_const_t<Type>>>
struct storage_for {
  using type = constness_as_t<
      storage_type_t<std::remove_const_t<Type>, Entity, Allocator>, Type>;
};

/**
 * \brief ヘルパー型
 */
template <typename... Args>
using storage_for_t = typename storage_for<Args...>::type;

template <typename Entity = Entity, typename = std::allocator<Entity>>
class BasicRegistry;

template <typename, typename, typename = void>
class basic_view;

template <typename, typename, typename>
class basic_group;

template <typename... Type>
using exclude_t = type_list<Type...>;

template <typename... Type>
inline constexpr exclude_t<Type...> exclude{};

template <typename... Type>
using get_t = type_list<Type...>;

template <typename... Type>
inline constexpr get_t<Type...> get{};

template <typename... Type>
using owned_t = type_list<Type...>;

template <typename... Type>
inline constexpr owned_t<Type...> owned{};

using sparse_set = basic_sparse_set<Entity>;

template <typename Type>
using storage = basic_storage<Type>;

using registry = BasicRegistry<Entity>;

template <typename Get, typename Exclude = exclude_t<>>
using view = basic_view<type_list_transform_t<Get, storage_for>,
                        type_list_transform_t<Exclude, storage_for>>;

template <typename Owned, typename Get, typename Exclude>
using group = basic_group<type_list_transform_t<Owned, storage_for>,
                          type_list_transform_t<Get, storage_for>,
                          type_list_transform_t<Exclude, storage_for>>;

}  // namespace becs

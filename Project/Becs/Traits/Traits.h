// @Traits.h
// @brief
// @author ICE
// @date 2023/03/02
//
// @details

#pragma once
#include <cstdint>
#include <bit>

namespace becs {

namespace internal {

template <typename, typename = void>
struct BecsTraits;

template <typename Type>
struct BecsTraits<Type, std::enable_if_t<std::is_enum_v<Type>>>
    : BecsTraits<std::underlying_type_t<Type>> {
  using value_type = Type;
};

template <typename Type>
struct BecsTraits<Type, std::enable_if_t<std::is_class_v<Type>>>
    : BecsTraits<typename Type::entity_type> {
  using value_type = Type;
};

template <>
struct BecsTraits<uint16_t> {
  using value_type = uint16_t;

  using EntityType = uint16_t;
  using VersionType = uint8_t;

  /*! @brief ハンドルID からエンティティ番号を取得するためのマスク */
  static constexpr uint16_t entity_mask = 0xFFF;
  /*! @brief バージョンを取得するためのマスク */
  static constexpr uint16_t version_mask = 0xF;
  /*! @brief エンティティ番号のビット範囲 */
  static constexpr auto entity_shift = 12;
};

/**
 * @brief 32 ビットのエンティティ識別子の特性
 *
 * 32 ビットエンティティの性質
 *
 * * エンティティ番号が 20 ビット割り当てられる
 * * バージョン番号が 12 ビット割り当てられる
 * ([0-4095]の範囲内で超えると 0 にリセットされる).
 */
template <>
struct BecsTraits<uint32_t> {
  using value_type = uint32_t;

  using EntityType = uint32_t;
  using VersionType = uint16_t;

  /*! @brief ハンドルID からエンティティ番号を取得するためのマスク */
  static constexpr uint32_t entity_mask = 0xFFFFF;
  /*! @brief バージョンを取得するためのマスク */
  static constexpr uint32_t version_mask = 0xFFF;
  /*! @brief エンティティ番号のビット範囲 */
  static constexpr auto entity_shift = 20;
};

/**
 * @brief 64 ビットのエンティティ識別子の特性
 *
 * 64 ビットエンティティの性質
 *
 * *  エンティティ番号が 32 ビット割り当てられる
 * *  バージョン番号が 32 ビット割り当てられる
 */
template <>
struct BecsTraits<uint64_t> {
  using value_type = uint64_t;

  using EntityType = uint64_t;
  using VersionType = uint32_t;
  using DifferenceType = int64_t;

  /*! @brief ハンドルID からエンティティ番号を取得するためのマスク */
  static constexpr uint64_t entity_mask = 0xFFFFFFFF;
  /*! @brief バージョンを取得するためのマスク */
  static constexpr uint64_t version_mask = 0xFFFFFFFF;
  /*! @brief エンティティ番号のビット範囲 */
  static constexpr auto entity_shift = 32;
};
}  // namespace internal

template <typename Traits>
class basic_BecsTraits {
  static constexpr auto length = std::popcount(Traits::entity_mask);

  static_assert(Traits::entity_mask &&
                    ((typename Traits::EntityType{1} << length) ==
                     (Traits::entity_mask + 1)),
                "Invalid Entity mask");
  static_assert((typename Traits::EntityType{1}
                 << std::popcount(Traits::version_mask)) ==
                    (Traits::version_mask + 1),
                "Invalid version mask");

 public:
  using value_type = typename Traits::value_type;
  using EntityType = typename Traits::EntityType;
  using VersionType = typename Traits::VersionType;

  static constexpr EntityType reserved =
      Traits::entity_mask | (Traits::version_mask << Traits::entity_shift);
  static constexpr EntityType entity_mask = Traits::entity_mask;
  static constexpr EntityType version_mask = Traits::version_mask;

  /**
   * @brief Entityを異なる型に変換する
   * @param value 変換する値
   * @return 指定された値の整数表現
   */
  [[nodiscard]] static constexpr EntityType to_integral(
      const value_type value) noexcept {
    return static_cast<EntityType>(value);
  }

  /**
   * @brief 基本型に変換されたエンティティ部分を返します
   * @param value 変換する値
   * @return エンティティ部分の整数表現
   */
  [[nodiscard]] static constexpr EntityType to_entity(
      const value_type value) noexcept {
    return (to_integral(value) & entity_mask);
  }

  /**
   * @brief 基になる型に変換されたバージョン部分を返します
   * @param value 変換する値
   * @return バージョン部分の整数表現
   */
  [[nodiscard]] static constexpr VersionType to_version(
      const value_type value) noexcept {
    return (to_integral(value) >> length);
  }

  /**
   * @brief 指定した識別子の次世代を返します.
   * @param value 次世代を返す識別子
   * @return 指定された識別子の次世代
   */
  [[nodiscard]] static constexpr value_type next(
      const value_type value) noexcept {
    const auto vers = to_version(value) + 1;
    return construct(to_entity(value),
                     static_cast<VersionType>(vers + (vers == version_mask)));
  }

  /**
   * @brief 識別子を構築する
   *
   * Entity 部分を指定されていない場合は null を返します。<br/>
   * version 部分を指定されていない場合は tombstone を返します。
   *
   * @param entity 識別子のエンティティ部分
   * @param version 識別子のバージョン部分
   * @return 適切に構築された識別子
   */
  [[nodiscard]] static constexpr value_type construct(
      const EntityType entity, const VersionType version) noexcept {
    return value_type{(entity & entity_mask) |
                      (static_cast<EntityType>(version) << length)};
  }

  /**
   * @brief 2 つの識別子を 1 つの識別子に結合します。
   *
   * @param lhs エンティティ部分の取得元の識別子
   * @param rhs バージョン部分を取得する識別子
   * @return 適切に構築された識別子
   */
  [[nodiscard]] static constexpr value_type combine(
      const EntityType lhs, const EntityType rhs) noexcept {
    constexpr auto mask = (version_mask << length);
    return value_type{(lhs & entity_mask) | (rhs & mask)};
  }
};
template <typename Type>
struct BecsTraits : basic_BecsTraits<internal::BecsTraits<Type>> {
  using base_type = basic_BecsTraits<internal::BecsTraits<Type>>;
  static constexpr std::size_t page_size = 32;
};

template <typename Entity>
[[nodiscard]] constexpr typename BecsTraits<Entity>::EntityType to_integral(
    const Entity value) noexcept {
  return BecsTraits<Entity>::to_integral(value);
}

template <typename Entity>
[[nodiscard]] constexpr typename BecsTraits<Entity>::EntityType to_entity(
    const Entity value) noexcept {
  return BecsTraits<Entity>::to_entity(value);
}

template <typename Entity>
[[nodiscard]] constexpr typename BecsTraits<Entity>::VersionType to_version(
    const Entity value) noexcept {
  return BecsTraits<Entity>::to_version(value);
}

struct null_t {
  /**
   * @brief null オブジェクトを任意の型の識別子に変換します
   * @tparam Entity 識別子型
   * @return 指定された型の null 表現
   */
  template <typename Entity>
  [[nodiscard]] constexpr operator Entity() const noexcept {
    using traits_type = BecsTraits<Entity>;
    constexpr auto value = traits_type::construct(traits_type::entity_mask,
                                                  traits_type::version_mask);
    return value;
  }

  [[nodiscard]] constexpr bool operator==(
      [[maybe_unused]] const null_t other) const noexcept {
    return true;
  }
  [[nodiscard]] constexpr bool operator!=(
      [[maybe_unused]] const null_t other) const noexcept {
    return false;
  }

  template <typename Entity>
  [[nodiscard]] constexpr bool operator==(const Entity entity) const noexcept {
    using traits_type = BecsTraits<Entity>;
    return traits_type::to_entity(entity) == traits_type::to_entity(*this);
  }

  template <typename Entity>
  [[nodiscard]] constexpr bool operator!=(const Entity entity) const noexcept {
    return !(entity == *this);
  }
};

template <typename Entity>
[[nodiscard]] constexpr bool operator==(const Entity entity,
                                        const null_t other) noexcept {
  return other.operator==(entity);
}

template <typename Entity>
[[nodiscard]] constexpr bool operator!=(const Entity entity,
                                        const null_t other) noexcept {
  return !(other == entity);
}

/**
 * \brief 削除済みのEntityのことを表す
 * Entityは削除されるとIDが再利用されます。
 * そのため、IDを保持していると誤って古い情報にアクセスしてしまう可能性がある。
 * 古い参照が tombstone_t
 * と指すようにすることで削除済みのEntityであることを表す。
 */
struct tombstone_t {
  /**
   * \brief 任意の識別子型に変換する
   * \tparam Entity 識別子型
   * \return 識別子型のtombstone表現
   */
  template <typename Entity>
  [[nodiscard]] constexpr operator Entity() const noexcept {
    using traits_type = BecsTraits<Entity>;
    constexpr auto value = traits_type::construct(traits_type::entity_mask,
                                                  traits_type::version_mask);
    return value;
  }

  /**
   * \brief tombstoneは状態を持たないため常に一致する
   * \param other 他のtombstoneオブジェクト
   * \return true
   */
  [[nodiscard]] constexpr bool operator==(
      [[maybe_unused]] const tombstone_t other) const noexcept {
    return true;
  }

  /**
   * @brief tombstoneは状態を持たないため違うことがない
   * @param other 他のtombstoneオブジェクト
   * @return false
   */
  [[nodiscard]] constexpr bool operator!=(
      [[maybe_unused]] const tombstone_t other) const noexcept {
    return false;
  }

  /**
   * @brief Compares a tombstone object and an identifier of any type.
   * @tparam Entity 識別子型.
   * @param entity 比較対象.
   * @return 二つの要素が一致しなければ false
   *         一致すれば true
   */
  template <typename Entity>
  [[nodiscard]] constexpr bool operator==(const Entity entity) const noexcept {
    using traits_type = BecsTraits<Entity>;
    return traits_type::to_version(entity) == traits_type::to_version(*this);
  }

  template <typename Entity>
  [[nodiscard]] constexpr bool operator!=(const Entity entity) const noexcept {
    return !(entity == *this);
  }
};

template <typename Entity>
[[nodiscard]] constexpr bool operator==(const Entity entity,
                                        const tombstone_t other) noexcept {
  return other.operator==(entity);
}

template <typename Entity>
[[nodiscard]] constexpr bool operator!=(const Entity entity,
                                        const tombstone_t other) noexcept {
  return !(other == entity);
}

/**
 * \brief コンパイル時定数
 *
 */
inline constexpr null_t null{};

/**
 * \brief コンパイル時定数
 * 破棄されたEntityオブジェクトを表す
 */
inline constexpr tombstone_t tombstone{};

}  // namespace becs
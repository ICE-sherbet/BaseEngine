// @UUID.h
// @brief 一意の識別子
// @author ICE
// @date 2023/03/07
//
// @details UUIDやGUIDは 128 bitを用いられる事がよくあるが
// 衝突の可能性が低いため 64 bitを使用。
// 将来的に変更される可能性が高いです。

#pragma once
#include <cstdint>
#include <functional>

namespace base_engine {

/**
 * \brief 64 bitの整数で表される一意の識別子
 * 衝突の可能性が低いため使用していますが、将来的に変更される可能性があります。
 */
class UUID {
 public:
  using ValueType = uint64_t;
  UUID();
  constexpr UUID(const ValueType uuid) : uuid_(uuid) {}

  constexpr operator ValueType() const { return uuid_; }

 private:
  ValueType uuid_ = 0;
};

class UUID32 {
 public:
  using ValueType = uint32_t;

  UUID32();
  constexpr UUID32(const ValueType uuid) : uuid_(uuid) {}

  constexpr operator ValueType() const { return uuid_; }

 private:
  ValueType uuid_ = 0;
};

constexpr UUID kNullUuid = UUID{0};
constexpr UUID32 kNullUuid32 = 0;
}  // namespace base_engine

namespace std {

template <>
struct hash<base_engine::UUID> {
  size_t operator()(const base_engine::UUID& uuid) const noexcept {
    return uuid;
  }
};

template <>
struct hash<base_engine::UUID32> {
  size_t operator()(const base_engine::UUID32& uuid) const noexcept {
    return hash<uint32_t>()(uuid);
  }
};
}  // namespace std

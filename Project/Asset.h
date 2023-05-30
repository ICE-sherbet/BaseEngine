// @Asset.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include "AssetTypes.h"
#include "Ref.h"
#include "UUID.h"

namespace base_engine {

using AssetHandle = UUID;

class Asset : public RefCounted {
 public:
  AssetHandle handle_ = kNullUuid;
  uint16_t flags_ = static_cast<uint16_t>(AssetFlag::kNone);
  Asset() = default;
  virtual ~Asset() = default;

  static AssetType GetStaticType() { return AssetType::kNone; }
  virtual AssetType GetAssetType() const { return AssetType::kNone; }

  bool IsValid() const {
    return ((flags_ & static_cast<uint16_t>(AssetFlag::kMissing)) |
            (flags_ & static_cast<uint16_t>(AssetFlag::kInvalid))) == 0;
  }

  virtual bool operator==(const Asset& other) const {
    return handle_ == other.handle_;
  }

  virtual bool operator!=(const Asset& other) const {
    return !(*this == other);
  }

  bool IsFlagSet(AssetFlag flag) const {
    return static_cast<uint16_t>(flag) & flags_;
  }
  void SetFlag(AssetFlag flag, const bool value = true) {
    if (value)
      flags_ |= static_cast<uint16_t>(flag);
    else
      flags_ &= ~static_cast<uint16_t>(flag);
  }
};
}  // namespace base_engine

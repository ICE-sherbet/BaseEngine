// @Script.h
// @brief
// @author ICE
// @date 2023/04/07
//
// @details

#pragma once
#include "Asset.h"
#include "AssetTypes.h"
namespace base_engine {
class Script : public Asset {
public:
  Script() = default;
  Script(const uint32_t classID) : class_id_(classID) {}

  uint32_t GetClassID() const { return class_id_; }

  static AssetType GetStaticType() { return AssetType::kScript; }
  virtual AssetType GetAssetType() const override { return GetStaticType(); }

 private:
  uint32_t class_id_ = 0;
};
}  // namespace base_engine

// @BodyTypeTag.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once

namespace base_engine::physics {

enum BodyType { kBodyTypeNone, kDynamic, kStatic };
template <typename T>
struct BodyTypeTag {
  static constexpr size_t Type();
};

struct Dynamic {
  static constexpr size_t Type() { return kDynamic; }

 private:
  int mass;
};
struct Static {
  static constexpr size_t Type() { return kStatic; }

 private:
  int mass;
};
}  // namespace base_engine::physics

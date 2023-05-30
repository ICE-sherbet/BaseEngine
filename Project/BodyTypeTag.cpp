#include "BodyTypeTag.h"

namespace base_engine::physics {
template <typename T>
constexpr size_t BodyTypeTag<T>::Type() {
  return T::Type();
}

template struct BodyTypeTag<Dynamic>;
template struct BodyTypeTag<Static>;
}  // namespace base_engine::physics

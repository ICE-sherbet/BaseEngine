#include "QuaternionUtilities.h"

namespace base_engine {

Quaternion Rotation(const Vector3& v) {
  Quaternion result;
  Mof::CQuaternionUtilities::Rotation(v, result);
  return result;
}
}  // namespace base_engine

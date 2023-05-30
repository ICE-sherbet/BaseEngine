// @QuaternionUtilities.inl
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include <algorithm>

#include "Vector3.h"

namespace base_engine {
inline Vector3 EulerAngles(const Quaternion& x) {
  return Vector3(Pitch(x), Yaw(x), Roll(x));
}
Floating Roll(const Quaternion& q) {
  return static_cast<Floating>(
      atan2(static_cast<Floating>(2) * (q.x * q.y + q.w * q.z),
            q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z));
};
Floating Pitch(const Quaternion& q) {
  Floating const y = static_cast<Floating>(2) * (q.y * q.z + q.w * q.x);
  Floating const x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;

  if (x == 0 && y == 0)
    return static_cast<Floating>(static_cast<Floating>(2) * atan2(q.x, q.w));

  return static_cast<Floating>(atan2(y, x));
}
Floating Yaw(const Quaternion& q) {
  return asin(std::clamp(static_cast<Floating>(-2) * (q.x * q.z - q.w * q.y),
                         static_cast<Floating>(-1), static_cast<Floating>(1)));
}

inline Matrix33 mat3_cast(Quaternion const& q) {
  Matrix33 Result{};
  Floating qxx(q.x * q.x);
  Floating qyy(q.y * q.y);
  Floating qzz(q.z * q.z);
  Floating qxz(q.x * q.z);
  Floating qxy(q.x * q.y);
  Floating qyz(q.y * q.z);
  Floating qwx(q.w * q.x);
  Floating qwy(q.w * q.y);
  Floating qwz(q.w * q.z);

  Result.rc[0][0] = Floating(1) - Floating(2) * (qyy + qzz);
  Result.rc[0][1] = Floating(2) * (qxy + qwz);
  Result.rc[0][2] = Floating(2) * (qxz - qwy);

  Result.rc[1][0] = Floating(2) * (qxy - qwz);
  Result.rc[1][1] = Floating(1) - Floating(2) * (qxx + qzz);
  Result.rc[1][2] = Floating(2) * (qyz + qwx);

  Result.rc[2][0] = Floating(2) * (qxz + qwy);
  Result.rc[2][1] = Floating(2) * (qyz - qwx);
  Result.rc[2][2] = Floating(1) - Floating(2) * (qxx + qyy);
  return Result;
}
}  // namespace base_engine

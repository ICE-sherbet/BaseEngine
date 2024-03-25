#include "QuaternionUtilities.h"

#include <glm/gtx/quaternion.hpp>

namespace base_engine {

Quaternion Rotation(const Vector3& v) {
  const auto cx = std::cos(0.5f * v.x);
  const auto sx = std::sin(0.5f * v.x);
  const auto cy = std::cos(0.5f * v.y);
  const auto sy = std::sin(0.5f * v.y);
  const auto cz = std::cos(0.5f * v.z);
  const auto sz = std::sin(0.5f * v.z);
  return {
	  -cx * sy * sz + sx * cy * cz, cx * sy * cz + sx * cy * sz,
                    sx * sy * cz + cx * cy * sz, -sx * sy * sz + cx * cy * cz
  };

}
}  // namespace base_engine

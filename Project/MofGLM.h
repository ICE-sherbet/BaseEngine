// @MofGLM.h
// @brief
// @author ICE
// @date 2024/03/12
//
// @details

#pragma once
#include <Math/Matrix44.h>
#include <Math/Vector3.h>

#include <glm/detail/type_mat4x4.hpp>
#include <glm/glm.hpp>

namespace base_engine::Mof2GLM {
static inline glm::vec3 Convert(const Mof::Vector3& vec) {
  return {vec.x, vec.y, vec.z};
}
static inline glm::mat4x4 Convert(const Mof::Matrix44& mat) {
  glm::mat4x4 result;
  memmove(&result, &mat, sizeof(Mof::Matrix44));
  return result;
}

}  // namespace base_engine::Mof2GLM

// @MeshImporterUtilities.h
// @brief
// @author ICE
// @date 2024/03/09
//
// @details

#pragma once
#include <assimp/matrix4x4.h>

#include <glm/glm.hpp>
#include <glm/glm/detail/type_quat.hpp>

namespace base_engine::utilities {
inline glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix) {
  glm::mat4 result;
  result[0][0] = matrix.a1;
  result[1][0] = matrix.a2;
  result[2][0] = matrix.a3;
  result[3][0] = matrix.a4;
  result[0][1] = matrix.b1;
  result[1][1] = matrix.b2;
  result[2][1] = matrix.b3;
  result[3][1] = matrix.b4;
  result[0][2] = matrix.c1;
  result[1][2] = matrix.c2;
  result[2][2] = matrix.c3;
  result[3][2] = matrix.c4;
  result[0][3] = matrix.d1;
  result[1][3] = matrix.d2;
  result[2][3] = matrix.d3;
  result[3][3] = matrix.d4;
  return result;
}

inline float AngleAroundYAxis(const glm::quat& quat) {
  static glm::vec3 x_axis = {1.0f, 0.0f, 0.0f};
  static glm::vec3 y_axis = {0.0f, 1.0f, 0.0f};
  const auto rotated_orthogonal = quat * x_axis;

  const auto projected = glm::normalize(
      rotated_orthogonal - (y_axis * glm::dot(rotated_orthogonal, y_axis)));
  return acos(glm::dot(x_axis, projected));
}
}  // namespace base_engine::utilities
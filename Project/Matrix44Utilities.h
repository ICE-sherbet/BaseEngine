// @Matrix44Utilities.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "Matrix44.h"
#include "Vector3.h"

namespace base_engine::mat {
inline Matrix44 Translate(const Matrix44& m, const Vector3& v) {
  Matrix44 result(m);

  result.rc[3][0] =
      m.rc[0][0] * v[0] + m.rc[1][0] * v[1] + m.rc[2][0] * v[2] + m.rc[3][0];
  result.rc[3][1] =
      m.rc[0][1] * v[0] + m.rc[1][1] * v[1] + m.rc[2][1] * v[2] + m.rc[3][1];
  result.rc[3][2] =
      m.rc[0][2] * v[0] + m.rc[1][2] * v[1] + m.rc[2][2] * v[2] + m.rc[3][2];
  result.rc[3][3] =
      m.rc[0][3] * v[0] + m.rc[1][3] * v[1] + m.rc[2][3] * v[2] + m.rc[3][3];
  return result;
};
inline Matrix44 Scale(const Matrix44& m, const Vector3& v) {
  Matrix44 result;
  const auto m0 = Mof::Vector4{const_cast<LPMofFloat>(m.rc[0])} * v[0];
  result.rc[0][0] = m0.fv[0];
  result.rc[0][1] = m0.fv[1];
  result.rc[0][2] = m0.fv[2];
  result.rc[0][3] = m0.fv[3];

  const auto m1 = Mof::Vector4{const_cast<LPMofFloat>(m.rc[1])} * v[1];
  result.rc[1][0] = m1.fv[0];
  result.rc[1][1] = m1.fv[1];
  result.rc[1][2] = m1.fv[2];
  result.rc[1][3] = m1.fv[3];

  const auto m2 = Mof::Vector4{const_cast<LPMofFloat>(m.rc[2])} * v[2];
  result.rc[2][0] = m2.fv[0];
  result.rc[2][1] = m2.fv[1];
  result.rc[2][2] = m2.fv[2];
  result.rc[2][3] = m2.fv[3];

  result.rc[3][0] = m.rc[3][0];
  result.rc[3][1] = m.rc[3][1];
  result.rc[3][2] = m.rc[3][2];
  result.rc[3][3] = m.rc[3][3];
  return result;
};
Matrix44 Inverse(const Matrix44& m);
}  // namespace base_engine::mat
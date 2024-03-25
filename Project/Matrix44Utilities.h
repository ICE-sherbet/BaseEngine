// @Matrix44Utilities.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "Matrix44.h"
#include "Vector3.h"
#include "Vector4.h"

namespace base_engine::mat{

#if defined(_USE_MOF_LIBRARY_)

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

#elif defined(_USE_GLM_LIBRARY_)

inline Matrix44 Translate(const Matrix44& m, const Vector3& v) {
  Matrix44 result(m);

  result[3][0] =
      m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0];
  result[3][1] =
      m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1];
  result[3][2] =
      m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2];
  result[3][3] =
      m[0][3] * v[0] + m[1][3] * v[1] + m[2][3] * v[2] + m[3][3];
  return result;
};
inline Matrix44 Scale(const Matrix44& m, const Vector3& v) {
  Matrix44 result;
  const auto m0 = m[0] * v[0];
  result[0][0] = m0[0];
  result[0][1] = m0[1];
  result[0][2] = m0[2];
  result[0][3] = m0[3];

  const auto m1 = m[1] * v[1];
  result[1][0] = m1[0];
  result[1][1] = m1[1];
  result[1][2] = m1[2];
  result[1][3] = m1[3];

  const auto m2 = m[2] * v[2];
  result[2][0] = m2[0];
  result[2][1] = m2[1];
  result[2][2] = m2[2];
  result[2][3] = m2[3];

  result[3][0] = m[3][0];
  result[3][1] = m[3][1];
  result[3][2] = m[3][2];
  result[3][3] = m[3][3];
  return result;
};

#endif

Matrix44 Inverse(const Matrix44& m);

}  // namespace base_engine::mat
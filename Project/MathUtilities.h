﻿// @MathUtilities.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "Matrix44.h"
#include "Quaternion.h"
#include "Vector3.h"

namespace base_engine::math {
inline Vector3 Scale(const Vector3& v, float desiredLength) {
  return v * desiredLength / Mof::CVector3Utilities::Length(v);
}

inline bool DecomposeTransform(const Matrix44& transform, Vector3& translation,
                               Quaternion& rotation, Vector3& scale) {
  using T = float;

  Matrix44 LocalMatrix(transform);
  if (std::abs(LocalMatrix.rc[3][3]) < std::numeric_limits<T>::epsilon())
    return false;

  translation = Vector3(LocalMatrix.rc[3]);
  LocalMatrix.rc[3][0] = 0;
  LocalMatrix.rc[3][1] = 0;
  LocalMatrix.rc[3][2] = 0;
  LocalMatrix.rc[3][3] = LocalMatrix._44;

  Vector3 Row[3];

  for (size_t i = 0; i < 3; ++i)
    for (size_t j = 0; j < 3; ++j) Row[i][j] = LocalMatrix.rc[i][j];

  scale.x = Mof::CVector3Utilities::Length(Row[0]);
  Row[0] = Scale(Row[0], static_cast<T>(1));

  scale.y = Mof::CVector3Utilities::Length(Row[1]);
  Row[1] = Scale(Row[1], static_cast<T>(1));
  scale.z = Mof::CVector3Utilities::Length(Row[2]);
  Row[2] = Scale(Row[2], static_cast<T>(1));

  int i, j, k = 0;
  T root, trace = Row[0].x + Row[1].y + Row[2].z;
  if (trace > static_cast<T>(0)) {
    root = sqrt(trace + static_cast<T>(1));
    rotation.w = static_cast<T>(0.5) * root;
    root = static_cast<T>(0.5) / root;
    rotation.x = root * (Row[2].y - Row[1].z);
    rotation.y = root * (Row[0].z - Row[2].x);
    rotation.z = root * (Row[1].x - Row[0].y);
  } else {
    static int Next[3] = {1, 2, 0};
    i = 0;
    if (Row[1].y > Row[0].x) i = 1;
    if (Row[2].z > Row[i][i]) i = 2;
    j = Next[i];
    k = Next[j];

    root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

    rotation[i] = static_cast<T>(0.5) * root;
    root = static_cast<T>(0.5) / root;
    rotation[j] = root * (Row[i][j] + Row[j][i]);
    rotation[k] = root * (Row[i][k] + Row[k][i]);
    rotation.w = root * (Row[j][k] - Row[k][j]);
  }

  return true;
}
}  // namespace base_engine::math

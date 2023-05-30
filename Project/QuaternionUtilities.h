// @QuaternionUtilities.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "Vector3.h"
#include "Matrix33.h"
#include "Matrix44.h"
#include "Quaternion.h"

namespace base_engine {
/**
 * \brief オイラー角を返します。ピッチは x、ヨーは y、ロールは z です。
 * 結果はラジアンで表されます。
 */
inline Vector3 EulerAngles(const Quaternion& x);

/**
 * \brief ベクトルの回転クオータニオンを求める
 * \param v 回転ベクトル
 * \return クォータニオン
 */
Quaternion Rotation(const Vector3& v);

/**
 * \brief ラジアンで表されたオイラー角のロール値を返します
 */
inline Floating Roll(const Quaternion& x);

/**
 * \brief ラジアンで表されたオイラー角のピッチ値を返します
 */
inline Floating Pitch(const Quaternion& x);

/**
 * \brief ラジアンで表されたオイラー角のヨー値を返します。
 */
inline Floating Yaw(const Quaternion& x);

inline Matrix33 mat3_cast(Quaternion const& q);

inline Matrix44 mat4_cast(Quaternion const& q)
{
  Matrix44 mat;
  Mof::CQuaternionUtilities::ConvertMatrix(q, mat);
	return mat;
}
inline Matrix44 ToMat4(Quaternion const& x) { return mat4_cast(x); }

}  // namespace base_engine

#include "QuaternionUtilities.inl"
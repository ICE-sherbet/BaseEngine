// @VelocityComponent.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include "Vector2.h"

namespace base_engine::physics {
struct VelocityComponent {
  /**
   * \brief 線形速度\n
   * 物体が移動する速さと方向を表すベクトル量
   */
  Vector2 linear;

  /**
   * \brief 物体に働く外力を表すベクトル量
   */
  Vector2 force;

  /**
   * \brief 角速度\n
   * 物体が回転する速さを表す量
   */
  float angular;

  /**
   * \brief ねじりモーメント\n
   * 回転軸に働く力
   */
  float torque;
};

}  // namespace base_engine::physics

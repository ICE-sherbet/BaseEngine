// @RigidBodyComponent.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once

#include "Vector2.h"

namespace base_engine::physics {
struct RigidBodyComponent {
  /**
   * \brief 反発係数
   */
  float restitution = 0;

  /**
   * \brief 質量
   */
  float mass = 1;

  /**
   * \brief 質量の逆数\n
   * 物体に働く外力から加速度を求めるのに利用する\n
   * ma = F 直線運動の運動方程式より
   */
  float inverse_mass = 1;

  /**
   * \brief 質量慣性モーメントの逆数。\n
   * 回転軸に働く力から角加速度を求めるのに利用する。\n
   * Iα = T 回転運動の運動方程式より
   */
  float inverse_MMOI = 1;

  RigidBodyComponent(const float restitution, const float mass, float mmoi)
      : restitution(restitution),
        mass(mass),
        inverse_mass(1.0f / mass),
        inverse_MMOI(1.0f / mmoi) {}

  RigidBodyComponent() = default;
};

}  // namespace base_engine::physics
// @SolveContact.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once
#include "Vector2.h"

namespace base_engine::physics {
template <typename TagA, typename TagB>
struct SolveContact {
  becs::Entity body_a;
  becs::Entity body_b;
  float penetration;
  Vector2 normal;  // body_bからbody_aへの法線
  Vector2 point;   // 貫通の中点

  SolveContact(const becs::Entity body_a, const becs::Entity body_b,
               float penetration, Vector2 normal, Vector2 point) {
    this->body_a = body_a;
    this->body_b = body_b;
    this->penetration = penetration;
    this->normal = normal;
    this->point = point;
  }
};
}  // namespace base_engine::physics
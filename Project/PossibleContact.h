// @PossibleContact.h
// @brief
// @author ICE
// @date 2023/05/09
//
// @details

#pragma once

namespace base_engine::physics {
template <typename ShapeA, typename ShapeB>
struct PossibleContact {
  becs::Entity body_a;
  becs::Entity body_b;
  int tag_a;
  int tag_b;

  PossibleContact(becs::Entity body_a, becs::Entity body_b, int tag_a,
                  int tag_b)
      : body_a(body_a), body_b(body_b), tag_a(tag_a), tag_b(tag_b) {}
};
}  // namespace base_engine::physics

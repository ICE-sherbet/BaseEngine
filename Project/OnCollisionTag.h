// @OnCollisionContactTag.h
// @brief
// @author ICE
// @date 2023/05/10
//
// @details

#pragma once

namespace base_engine::physics {
struct OnCollisionEnterTag {
  explicit OnCollisionEnterTag(const becs::Entity target) : target(target) {}

  becs::Entity target;
};
struct OnCollisionStayTag {
 private:
  int dummy_alignment;
};
}  // namespace base_engine::physics
// @BodyMask.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include <cstdint>

#include "ComponentDB.h"
#include "ComponentProperty.h"
namespace base_engine::physics {
struct BodyMask {
  BE_COMPONENT(BodyMask)

  int shape_type_id;
  int tag_type_id;
  uint32_t body_mask;
  uint32_t target_mask;
  BodyMask() {
    shape_type_id = 0;
    tag_type_id = 0;
    body_mask = 0xFFFFFFFFu;
    target_mask = 0xFFFFFFFFu;
  }

  BodyMask(const int shape_id, const int tag_id) {
    shape_type_id = shape_id;
    tag_type_id = tag_id;
    body_mask = 0xFFFFFFFFu;
    target_mask = 0xFFFFFFFFu;
  }

  BodyMask(const int shape_type_id, const int tag_type_id,
           const uint32_t body_mask, const uint32_t target_mask)
      : shape_type_id(shape_type_id),
        tag_type_id(tag_type_id),
        body_mask(body_mask),
        target_mask(target_mask) {}

  [[nodiscard]] uint32_t GetBodyMask() const { return body_mask; }
  void SetBodyMask(uint32_t body_mask) { this->body_mask = body_mask; }

  [[nodiscard]] uint32_t TargetMask() const { return target_mask; }
  void SetTargetMask(uint32_t target_mask) { this->target_mask = target_mask; }

  static void _Bind();
};
}  // namespace base_engine::physics

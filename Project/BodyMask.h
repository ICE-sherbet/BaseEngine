// @BodyMask.h
// @brief
// @author ICE
// @date 2023/05/08
//
// @details

#pragma once
#include <cstdint>
namespace base_engine::physics {
struct BodyMask {
  int shape_type_id;
  int tag_type_id;
  uint32_t body_mask;
  uint32_t target_mask;
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
};

}  // namespace base_engine::physics

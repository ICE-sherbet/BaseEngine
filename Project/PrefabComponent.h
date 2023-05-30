// @PrefabComponent.h
// @brief
// @author ICE
// @date 2023/05/29
//
// @details

#pragma once
#include "UUID.h"

namespace base_engine::component {
struct PrefabComponent {
  UUID prefab_id = 0;
  UUID entity_id = 0;
};

}  // namespace base_engine

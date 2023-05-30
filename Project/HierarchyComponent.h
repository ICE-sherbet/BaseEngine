// @HierarchyComponent.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "UUID.h"

namespace base_engine::component {
struct HierarchyComponent {
  UUID parent_handle = kNullUuid;
  std::vector<UUID> children;

  HierarchyComponent();
  HierarchyComponent(const HierarchyComponent& other);
  HierarchyComponent(const UUID& parent) : parent_handle(parent) {}
};

inline HierarchyComponent::HierarchyComponent() = default;

inline HierarchyComponent::HierarchyComponent(const HierarchyComponent& other) =
    default;
}  // namespace base_engine::component
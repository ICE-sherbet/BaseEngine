// @TagComponent.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include <string>

#include "ComponentDB.h"
#include "ComponentProperty.h"

namespace base_engine::component {
struct TagComponent {
  BE_COMPONENT(TagComponent)

  std::string tag = "None";

  TagComponent() = default;
  TagComponent(const TagComponent& other) = default;

  TagComponent& operator=(const TagComponent& other) {
    if (this == &other) return *this;
    tag = other.tag;
    return *this;
  }

  TagComponent(const std::string_view tag) : tag(tag) {}

  operator std::string&() { return tag; }
  operator const std::string&() const { return tag; }

  [[nodiscard]] std::string Tag() const { return tag; }
  void SetTag(const std::string& tag) { this->tag = tag; }

  static void _Bind();
};
}  // namespace base_engine::component

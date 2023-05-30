// @TagComponent.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include <string>

namespace base_engine::component {
struct TagComponent {
  std::string tag = "None";

  TagComponent() = default;
  TagComponent(const TagComponent& other) = default;
  TagComponent(TagComponent&& other) noexcept : tag(std::move(other.tag)) {}

  TagComponent& operator=(const TagComponent& other) {
    if (this == &other) return *this;
    tag = other.tag;
    return *this;
  }

  TagComponent& operator=(TagComponent&& other) noexcept {
    if (this == &other) return *this;
    tag = std::move(other.tag);
    return *this;
  }

  TagComponent(const std::string_view tag) : tag(tag) {}

  operator std::string&() { return tag; }
  operator const std::string&() const { return tag; }
};
}  // namespace base_engine::component

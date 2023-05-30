// @SpriteRendererComponent.h
// @brief
// @author ICE
// @date 2023/03/22
//
// @details

#pragma once
#include "Asset.h"
#include "Vector2.h"
#include "Vector4.h"

namespace base_engine::component {
struct SpriteRendererComponent {
  Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  AssetHandle texture = kNullUuid;
  Vector2 uv_start{0.0f, 0.0f};
  Vector2 uv_end{1.0f, 1.0f};
  Vector2 pivot{0.0f, 0.0f};

  float tiling_factor = 1.0f;

  SpriteRendererComponent() = default;
  SpriteRendererComponent(const SpriteRendererComponent& other) = default;
};

}  // namespace base_engine::component

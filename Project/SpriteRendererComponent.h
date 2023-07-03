// @SpriteRendererComponent.h
// @brief
// @author ICE
// @date 2023/03/22
//
// @details

#pragma once
#include "Asset.h"
#include "ComponentDB.h"
#include "ComponentProperty.h"
#include "Vector2.h"
#include "Vector4.h"

namespace base_engine::component {
struct SpriteRendererComponent {
  BE_COMPONENT(SpriteRendererComponent)

  Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  AssetHandle texture = kNullUuid;
  Vector2 uv_start{0.0f, 0.0f};
  Vector2 uv_end{1.0f, 1.0f};
  Vector2 pivot{0.0f, 0.0f};

  float tiling_factor = 1.0f;

  SpriteRendererComponent() = default;
  SpriteRendererComponent(const SpriteRendererComponent& other) = default;

  [[nodiscard]] Vector4 Color() const { return color; }
  void SetColor(const Vector4& color) { this->color = color; }

  [[nodiscard]] AssetHandle Texture() const { return texture; }
  void SetTexture(const AssetHandle& texture) { this->texture = texture; }

  [[nodiscard]] Vector2 UvStart() const { return uv_start; }
  void SetUvStart(const Vector2& uv_start) { this->uv_start = uv_start; }

  [[nodiscard]] Vector2 UvEnd() const { return uv_end; }
  void SetUvEnd(const Vector2& uv_end) { this->uv_end = uv_end; }

  [[nodiscard]] Vector2 Pivot() const { return pivot; }
  void SetPivot(const Vector2& pivot) { this->pivot = pivot; }

  [[nodiscard]] float TilingFactor() const { return tiling_factor; }
  void SetTilingFactor(float tiling_factor) {
    this->tiling_factor = tiling_factor;
  }

  static void _Bind();
};

}  // namespace base_engine::component

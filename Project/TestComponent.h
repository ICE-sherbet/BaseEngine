// @TestComponent.h
// @brief
// @author ICE
// @date 2023/06/25
//
// @details

#pragma once

#pragma once
#include "Asset.h"
#include "ComponentDB.h"
#include "ComponentProperty.h"
#include "Vector2.h"
#include "Vector4.h"

namespace base_engine::component {
struct TestComponent {
  BE_COMPONENT(TestComponent)

  Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  AssetHandle texture = kNullUuid;
  Vector2 uv_start{0.0f, 0.0f};
  Vector2 uv_end{1.0f, 1.0f};
  Vector2 pivot{0.0f, 0.0f};

  float tiling_factor = 1.0f;

  TestComponent() = default;
  TestComponent(const TestComponent& other) = default;

  [[nodiscard]] Vector4 Color() const { return color; }

  void SetColor(const Vector4& color) { this->color = color; }

  static void _Bind() {
    ComponentDB::BindMethod("SetColor", &TestComponent::SetColor);
    ComponentDB::BindMethod("GetColor", &TestComponent::Color);

    ADD_PROPERTY(PropertyInfo(VariantType::kVECTOR4F, "color"), "SetColor", "GetColor");

  }
};

}  // namespace base_engine::component

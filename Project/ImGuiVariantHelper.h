// @ImGuiVariantHelper.h
// @brief
// @author ICE
// @date 2023/05/24
//
// @details

#pragma once
#include <functional>
#include <string>

#include "ScriptTypes.h"
#include "imgui/imgui.h"

namespace base_engine::editor::ImGuiHelper {
inline void VariantField(const std::string& label, Variant* variant) {
  auto decorator = [variant]<class T>(auto func) {
    T value = static_cast<T>(*variant);
    func(&value);
    const Variant variant_fix{value};
    (*variant) = variant_fix;
  };
  switch (variant->GetType()) {
    case VariantType::kBool:
      decorator.operator()<bool>(
          [&label](bool* value) { ::ImGui::Checkbox(label.c_str(), value); });
      break;
    case VariantType::kInt8:
      decorator.operator()<int8_t>([&label](int8_t* value) {
        int buf = *value;
        ::ImGui::DragInt(label.c_str(), &buf, 1,
                         (std::numeric_limits<int8_t>::min)(),
                         (std::numeric_limits<int8_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt16:
      decorator.operator()<int16_t>([&label](int16_t* value) {
        int buf = *value;
        ::ImGui::DragInt(label.c_str(), &buf, 1,
                         (std::numeric_limits<int16_t>::min)(),
                         (std::numeric_limits<int16_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt32:
      decorator.operator()<int32_t>([&label](int32_t* value) {
        int buf = *value;
        ::ImGui::DragInt(label.c_str(), &buf, 1,
                         (std::numeric_limits<int32_t>::min)(),
                         (std::numeric_limits<int32_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt64:
      decorator.operator()<int64_t>([&label](int64_t* value) {
        int buf = *value;
        ::ImGui::DragInt(label.c_str(), &buf, 1,
                         (std::numeric_limits<int64_t>::min)(),
                         (std::numeric_limits<int64_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kUInt8:
      break;
    case VariantType::kUInt16:
      break;
    case VariantType::kUInt32:
      break;
    case VariantType::kUInt64:
      break;
    case VariantType::kFloat:
      decorator.operator()<float>([&label](float* value) {
        ::ImGui::DragFloat(label.c_str(), value, 1);
      });
      break;
    case VariantType::kDouble:
      decorator.operator()<double>([&label](double* value) {
        float buf = *value;
        ::ImGui::DragFloat(label.c_str(), &buf, 1);
        (*value) = buf;
      });
      break;
    case VariantType::kString:
      decorator.operator()<std::string>([&label](std::string* value) {
        char buf[256];
        ::ImGui::InputText(label.c_str(), buf, 256);
        (*value) = buf;
      });
      break;
    case VariantType::RECT2D:
      break;
    case VariantType::RECT2F:
      break;
    case VariantType::RECT2I:
      break;
    case VariantType::VECTOR2F:
      break;
    case VariantType::VECTOR3F:
      break;
    case VariantType::VECTOR4F:
      break;
    case VariantType::COLOR:
      break;
    case VariantType::MANAGED:
      break;
    default:;
  }
}

}  // namespace base_engine::editor::ImGuiHelper

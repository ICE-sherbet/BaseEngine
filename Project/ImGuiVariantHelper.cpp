#include "ImGuiVariantHelper.h"

#include "AssetManager.h"
#include "ImGuiAssetHelper.h"

namespace base_engine::editor {
bool ImGuiHelper::VariantField(const std::string& label, Variant* variant) {
  bool result = false;
  auto decorator = [variant]<class T>(auto func) {
    T value = static_cast<T>(*variant);
    func(&value);
    const Variant variant_fix{value};
    (*variant) = variant_fix;
  };
  switch (variant->GetType()) {
    case VariantType::kBool:
      decorator.operator()<bool>([&label, &result](bool* value) {
        result = ::ImGui::Checkbox(label.c_str(), value);
      });
      break;
    case VariantType::kInt8:
      decorator.operator()<int8_t>([&label, &result](int8_t* value) {
        int buf = *value;
        result = ::ImGui::DragInt(label.c_str(), &buf, 1,
                                  (std::numeric_limits<int8_t>::min)(),
                                  (std::numeric_limits<int8_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt16:
      decorator.operator()<int16_t>([&label, &result](int16_t* value) {
        int buf = *value;
        result = ::ImGui::DragInt(label.c_str(), &buf, 1,
                                  (std::numeric_limits<int16_t>::min)(),
                                  (std::numeric_limits<int16_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt32:
      decorator.operator()<int32_t>([&label, &result](int32_t* value) {
        int buf = *value;
        ::ImGui::DragInt(label.c_str(), &buf, 1,
                         (std::numeric_limits<int32_t>::min)(),
                         (std::numeric_limits<int32_t>::max)());
        *value = buf;
      });
      break;
    case VariantType::kInt64:
      decorator.operator()<int64_t>([&label, &result](int64_t* value) {
        int buf = *value;
        result = ::ImGui::DragInt(label.c_str(), &buf, 1,
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
      decorator.operator()<float>([&label, &result](float* value) {
        result = ::ImGui::DragFloat(label.c_str(), value, 1);
      });
      break;
    case VariantType::kDouble:
      decorator.operator()<double>([&label, &result](double* value) {
        float buf = *value;
        result = ::ImGui::DragFloat(label.c_str(), &buf, 1);
        (*value) = buf;
      });
      break;
    case VariantType::kString:
      decorator.operator()<std::string>([&label, &result](std::string* value) {
        char buf[256];
        result = ::ImGui::InputText(label.c_str(), buf, 256);
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
    case VariantType::kAssetHandle:
      decorator.operator()<AssetHandle>([&label, &result](AssetHandle* value) {
        const bool valid = AssetManager::IsAssetHandleValid(*value);
        const auto hint =
            valid ? AssetManager::GetEditorAssetManager()->GetAssetType(*value)
                  : AssetType::kNone;
        result = AssetReferenceField(label, value, hint);
      });
      break;
    case VariantType::MANAGED:
      break;
    default:;
  }
  return result;
}
}  // namespace base_engine::editor

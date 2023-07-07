#include "InspectorUtilities.h"

#include "EditorProperties.h"

namespace base_engine::editor::inspector {
std::shared_ptr<EditorProperty> MakeEditorProperty(
    void* object, const VariantType type, PropertyHint hint,
    const std::string& hint_name) {
  switch (type) {
    case VariantType::kNil:
      break;
    case VariantType::kVoid:
      break;
    case VariantType::kBool: {
      auto editor = std::make_shared<EditorPropertyCheck>();
      return editor;
    } break;
    case VariantType::kInt8: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<int8_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kInt16: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<int16_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kInt32: {
      if (hint == PropertyHint::kLayerPhysics) {
        auto editor = std::make_shared<EditorPropertyLayers>();
        return editor;
      } else {
        auto editor = std::make_shared<EditorPropertyInteger>();
        using Limit = std::numeric_limits<int32_t>;
        editor->Setup(Limit::min(), Limit::max(), 1);
        return editor;
      }
    } break;
    case VariantType::kInt64: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<int64_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kUInt8: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<uint8_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kUInt16: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<uint16_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kUInt32: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<uint32_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kUInt64: {
      auto editor = std::make_shared<EditorPropertyInteger>();
      using Limit = std::numeric_limits<uint64_t>;
      editor->Setup(Limit::min(), Limit::max(), 1);
      return editor;
    } break;
    case VariantType::kFloat: {
      auto editor = std::make_shared<EditorPropertyFloat>();
      return editor;
    } break;
    case VariantType::kDouble:
      break;
    case VariantType::kString:
      break;
    case VariantType::kRECT2D:
      break;
    case VariantType::kRECT2F:
      break;
    case VariantType::kRECT2I:
      break;
    case VariantType::kVECTOR2F: {
      auto editor = std::make_shared<EditorPropertyVector2>();
      return editor;
    } break;
    case VariantType::kVECTOR3F: {
      auto editor = std::make_shared<EditorPropertyVector3>();
      return editor;
    } break;
    case VariantType::kVECTOR4F:
      break;
    case VariantType::kCOLOR:
      break;
    case VariantType::kAssetHandle: {
      auto editor = std::make_shared<EditorPropertyAsset>();
      editor->Initialize(hint_name);
      return editor;
    } break;
    case VariantType::MANAGED:
      break;
    default:;
  }

  return nullptr;
}
}  // namespace base_engine::editor::inspector

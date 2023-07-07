#include "Variant.h"

#include <mono/metadata/object.h>

#include "CSharpScriptEngine.h"
#include "MonoScriptUtilities.h"
#include "ObjectEntity.h"
namespace base_engine {
template <typename T>
void CopyUnbox(T& value, MonoObject* obj) {
  value = *static_cast<T*>(mono_object_unbox(obj));
}

Variant::Variant() : data_() { this->type_ = VariantType::kNil; }

Variant::Variant(VariantType type) : data_() {
  this->type_ = type;

  switch (type) {
    case VariantType::kNil:
      break;
    case VariantType::kVoid:
      break;
    case VariantType::kBool:
      this->data_.bool_value = false;
      break;
    case VariantType::kInt8:
      this->data_.int8_value = 0;
      break;
    case VariantType::kInt16:
      this->data_.int16_value = 0;
      break;
    case VariantType::kInt32:
      this->data_.int32_value = 0;
      break;
    case VariantType::kInt64:
      this->data_.int64_value = 0;
      break;
    case VariantType::kUInt8:
      this->data_.uint8_value = 0;
      break;
    case VariantType::kUInt16:
      this->data_.uint16_value = 0;
      break;
    case VariantType::kUInt32:
      this->data_.uint32_value = 0;
      break;
    case VariantType::kUInt64:
      this->data_.uint64_value = 0;
      break;
    case VariantType::kFloat:
      this->data_.float_value = 0;
      break;
    case VariantType::kDouble:
      this->data_.double_value = 0;
      break;
    case VariantType::kString:
      new (this->data_.mem) std::string("");
      break;
    case VariantType::kRECT2D:
      break;
    case VariantType::kRECT2F:
      break;
    case VariantType::kRECT2I:
      break;
    case VariantType::kVECTOR2F:
      break;
    case VariantType::kVECTOR3F:
      break;
    case VariantType::kVECTOR4F:
      break;
    case VariantType::kCOLOR:
      break;
    case VariantType::MANAGED:
      break;
    default:;
  }
}

Variant::Variant(const bool value) {
  this->type_ = VariantType::kBool;
  data_.bool_value = value;
}

Variant::Variant(const int8_t value) {
  this->type_ = VariantType::kInt8;
  data_.int32_value = value;
}

Variant::Variant(int16_t value) {
  this->type_ = VariantType::kInt16;
  data_.int16_value = value;
}

Variant::Variant(const int32_t value) {
  this->type_ = VariantType::kInt32;
  data_.int32_value = value;
}

Variant::Variant(const int64_t value) {
  this->type_ = VariantType::kInt64;
  data_.int64_value = value;
}

Variant::Variant(const uint8_t value) {
  this->type_ = VariantType::kUInt8;
  data_.uint8_value = value;
}

Variant::Variant(const uint16_t value) {
  this->type_ = VariantType::kUInt16;
  data_.uint16_value = value;
}

Variant::Variant(const uint32_t value) {
  this->type_ = VariantType::kUInt32;
  data_.uint32_value = value;
}

Variant::Variant(const uint64_t value) {
  this->type_ = VariantType::kUInt64;
  data_.uint64_value = value;
}

Variant::Variant(const float value) {
  this->type_ = VariantType::kFloat;
  data_.float_value = value;
}

Variant::Variant(const double value) {
  this->type_ = VariantType::kDouble;
  data_.double_value = value;
}

Variant::Variant(const char* value) : data_() {
  this->type_ = VariantType::kString;
  new (data_.mem)(std::shared_ptr)(std::make_shared<std::string>(value));
}

Variant::Variant(const std::string& value) : data_() {
  this->type_ = VariantType::kString;
  new (data_.mem)(std::shared_ptr)(std::make_shared<std::string>(value));
}

Variant::Variant(const Vector2& value) : data_() {
  this->type_ = VariantType::kVECTOR2F;
  new (data_.mem) Vector2(value);
}

Variant::Variant(const Vector3& value) : data_() {
  this->type_ = VariantType::kVECTOR3F;
  new (data_.mem) Vector3(value);
}

Variant::Variant(const Vector4& value) : data_() {
  this->type_ = VariantType::kVECTOR4F;
  new (data_.mem) Vector4(value);
}

Variant::Variant(const AssetHandle value) {
  type_ = VariantType::kAssetHandle;
  data_.uuid_value = value;
}

Variant::Variant(MonoObject* object, const VariantType type) {
  this->type_ = type;
  switch (type) {
    case VariantType::kNil:
      break;
    case VariantType::kVoid:
      break;
    case VariantType::kBool:
      CopyUnbox(data_.bool_value, object);
      break;
    case VariantType::kInt8:
      CopyUnbox(data_.int8_value, object);
      break;
    case VariantType::kInt16:
      CopyUnbox(data_.int16_value, object);
      break;
    case VariantType::kInt32:
      CopyUnbox(data_.int32_value, object);
      break;
    case VariantType::kInt64:
      CopyUnbox(data_.int64_value, object);
      break;
    case VariantType::kUInt8:
      CopyUnbox(data_.uint8_value, object);
      break;
    case VariantType::kUInt16:
      CopyUnbox(data_.uint16_value, object);
      break;
    case VariantType::kUInt32:
      CopyUnbox(data_.uint32_value, object);
      break;
    case VariantType::kUInt64:
      CopyUnbox(data_.uint64_value, object);
      break;
    case VariantType::kFloat:
      CopyUnbox(data_.float_value, object);
      break;
    case VariantType::kDouble:
      CopyUnbox(data_.double_value, object);
      break;
    case VariantType::kString:
      break;
    case VariantType::kRECT2D:
      break;
    case VariantType::kRECT2F:
      break;
    case VariantType::kRECT2I:
      break;
    case VariantType::kVECTOR2F:
      break;
    case VariantType::kVECTOR3F:
      break;
    case VariantType::kVECTOR4F:
      break;
    case VariantType::kCOLOR:
      break;
    case VariantType::MANAGED:
      break;
    case VariantType::kAssetHandle: {
      //      CSharpScriptEngine::GetInstance()->GetMonoObjectClass()
      auto GetClass = [](const std::string& name) {
        return mono_class_from_name(
            CSharpScriptEngine::GetInstance()->GetCoreImage(),
            "BaseEngine_ScriptCore", name.c_str());
      };
      const auto clazz = mono_object_get_class(object);
      MonoScriptUtilities::ResolveMonoClassName(clazz);
      if (GetClass("Prefab") == clazz) {
        const auto prefab_handle_object =
            MonoScriptUtilities::GetFieldValueObject(object, "_Handle", false);
        CopyUnbox(data_.uuid_value, prefab_handle_object);
      }
    } break;
    default:;
  }
}

Variant::operator float() const { return data_.float_value; }

Variant::operator std::string() const {
  return **reinterpret_cast<const std::shared_ptr<std::string>*>(data_.mem);
}

Variant::operator Vector2() const {
  return *reinterpret_cast<const Vector2*>(data_.mem);
}

Variant::operator Vector3() const {
  return *reinterpret_cast<const Vector3*>(data_.mem);
}

Variant::operator Vector4() const {
  return *reinterpret_cast<const Vector4*>(data_.mem);
}
}  // namespace base_engine

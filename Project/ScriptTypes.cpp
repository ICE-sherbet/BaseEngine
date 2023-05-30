#include "ScriptTypes.h"

#include <mono/metadata/object.h>

namespace base_engine {
template <typename T>
void CopyUnbox(T& value, MonoObject* obj) {
  value = *static_cast<T*>(mono_object_unbox(obj));
}

Variant::Variant() {}

Variant::Variant(VariantType type) {
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

Variant::Variant(const std::string& value) {
  this->type_ = VariantType::kString;
  new (data_.mem) std::string(value);
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
    case VariantType::kAssetHandle:
      //CopyUnbox(data_.uuid_value, object);
      break;
    default:;
  }
}

Variant::operator float() const { return data_.float_value; }

Variant::operator std::string() const {
  return *reinterpret_cast<const std::string*>(data_.mem);
}
}  // namespace base_engine

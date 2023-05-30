// @ScriptTypes.h
// @brief
// @author ICE
// @date 2023/04/06
//
// @details

#pragma once

#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include "Asset.h"
#include "Ref.h"
#include "Vector2.h"

extern "C" {
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClassField MonoClassField;
typedef struct _MonoProperty MonoProperty;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoType MonoType;
typedef struct _MonoString MonoString;
}
namespace base_engine {

;

enum class VariantType {
  kNil,
  kVoid,
  kBool,
  kInt8,
  kInt16,
  kInt32,
  kInt64,
  kUInt8,
  kUInt16,
  kUInt32,
  kUInt64,

  kFloat,
  kDouble,

  kString,

  RECT2D,
  RECT2F,
  RECT2I,

  VECTOR2F,
  VECTOR3F,
  VECTOR4F,

  COLOR,

  kAssetHandle,

  MANAGED,
};

class Variant {
 public:
  Variant();
  explicit Variant(VariantType type);
  explicit Variant(bool value);
  explicit Variant(int8_t value);
  explicit Variant(int16_t value);
  explicit Variant(int32_t value);
  explicit Variant(int64_t value);
  explicit Variant(uint8_t value);
  explicit Variant(uint16_t value);
  explicit Variant(uint32_t value);
  explicit Variant(uint64_t value);
  explicit Variant(float value);
  explicit Variant(double value);
  explicit Variant(const std::string& value);
  explicit Variant(AssetHandle value);

  Variant(MonoObject* object, VariantType type);
  explicit operator bool() const { return data_.bool_value; }
  explicit operator int8_t() const { return data_.int8_value; }
  explicit operator int16_t() const { return data_.int16_value; }
  explicit operator int32_t() const { return data_.int32_value; }
  explicit operator int64_t() const { return data_.int64_value; }
  explicit operator uint8_t() const { return data_.uint8_value; }
  explicit operator uint16_t() const { return data_.uint16_value; }
  explicit operator uint32_t() const { return data_.uint32_value; }
  explicit operator uint64_t() const { return data_.uint64_value; }
  explicit operator float() const;
  explicit operator double() const { return data_.double_value; }
  explicit operator AssetHandle() const { return data_.uuid_value; }

  explicit operator std::string() const;

  void Visit(auto visitor);
  void Visit(auto visitor) const;

  VariantType GetType() const { return type_; }

 private:
  VariantType type_ = VariantType::kNil;
  union {
    bool bool_value;
    int8_t int8_value;
    int16_t int16_value;
    int32_t int32_value;
    int64_t int64_value;
    uint8_t uint8_value;
    uint16_t uint16_value;
    uint32_t uint32_value;
    uint64_t uint64_value;
    float float_value;
    double double_value;
    UUID uuid_value;
    uint8_t mem[8]{0};
  } data_ alignas(8);
};

void Variant::Visit(auto visitor) {
  switch (type_) {
    case VariantType::kNil:
      break;
    case VariantType::kVoid:
      break;
    case VariantType::kBool:
      visitor(data_.bool_value);
      break;
    case VariantType::kInt8:
      visitor(data_.int8_value);
      break;
    case VariantType::kInt16:
      visitor(data_.int16_value);
      break;
    case VariantType::kInt32:
      visitor(data_.int32_value);
      break;
    case VariantType::kInt64:
      visitor(data_.int64_value);
      break;
    case VariantType::kUInt8:
      visitor(data_.uint8_value);
      break;
    case VariantType::kUInt16:
      visitor(data_.uint16_value);
      break;
    case VariantType::kUInt32:
      visitor(data_.uint32_value);
      break;
    case VariantType::kUInt64:
      visitor(data_.uint64_value);
      break;
    case VariantType::kFloat:
      visitor(data_.float_value);
      break;
    case VariantType::kDouble:
      visitor(data_.double_value);
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
      visitor(data_.uuid_value);
      break;
    default:;
  }
}

void Variant::Visit(auto visitor) const {
  switch (type_) {
    case VariantType::kNil:
      break;
    case VariantType::kVoid:
      break;
    case VariantType::kBool:
      visitor(data_.bool_value);
      break;
    case VariantType::kInt8:
      visitor(data_.int8_value);
      break;
    case VariantType::kInt16:
      visitor(data_.int16_value);
      break;
    case VariantType::kInt32:
      visitor(data_.int32_value);
      break;
    case VariantType::kInt64:
      visitor(data_.int64_value);
      break;
    case VariantType::kUInt8:
      visitor(data_.uint8_value);
      break;
    case VariantType::kUInt16:
      visitor(data_.uint16_value);
      break;
    case VariantType::kUInt32:
      visitor(data_.uint32_value);
      break;
    case VariantType::kUInt64:
      visitor(data_.uint64_value);
      break;
    case VariantType::kFloat:
      visitor(data_.float_value);
      break;
    case VariantType::kDouble:
      visitor(data_.double_value);
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
      visitor(data_.uuid_value);
      break;
    default:;
  }
}

struct PropertyInfo {
  VariantType type = VariantType::kNil;
  std::string name;
  std::string class_name;
};
namespace {
consteval uint64_t Bit(const int n) { return std::rotl(1u, n); }
}  // namespace

enum class FieldFlags {
  kNone = -1,
  kReadOnly = Bit(0),
  kStatic = Bit(1),
  kPublic = Bit(2),
  kPrivate = Bit(3),
  kProtected = Bit(4),
  kInternal = Bit(5),
  kIsArray = Bit(6)
};

struct MethodInfo {
  enum MethodFlags {
    METHOD_FLAG_NORMAL = 1,
    METHOD_FLAG_EDITOR = 2,
    METHOD_FLAG_CONST = 4,
    METHOD_FLAG_VIRTUAL = 8,
    METHOD_FLAG_VARARG = 16,
    METHOD_FLAG_STATIC = 32,
    METHOD_FLAG_OBJECT_CORE = 64,
    METHOD_FLAGS_DEFAULT = METHOD_FLAG_NORMAL,
  };

  std::string name;
  PropertyInfo return_value;

  uint32_t flags = METHOD_FLAGS_DEFAULT;
  int id = 0;
};

}  // namespace base_engine

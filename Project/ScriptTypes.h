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
#include "Variant.h"
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

enum class PropertyHint
{
	kNone,
  kAsset,

  kLayerPhysics
};

struct PropertyInfo {
  uint32_t id = 0;
  VariantType type = VariantType::kNil;
  std::string name;
  PropertyHint hint = PropertyHint::kNone;
  std::string hint_name = "";
  std::string class_name;

  PropertyInfo(VariantType type = VariantType::kNil,
               const std::string& name = "unknown",
               PropertyHint hint = PropertyHint::kNone,
               const std::string& hint_name = "")
      : type(type), name(name), hint(hint), hint_name(hint_name) {}
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

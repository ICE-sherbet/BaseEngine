#include "MonoScriptUtilities.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>

#include "CSharpScriptEngine.h"
#include "ObjectEntity.h"
namespace base_engine {
std::string MonoScriptUtilities::ResolveMonoClassName(MonoClass* mono_class) {
  const char* class_name_ptr = mono_class_get_name(mono_class);
  std::string class_name = class_name_ptr != nullptr ? class_name_ptr : "";

  if (class_name.empty()) return "Unknown Class";

  if (const char* class_namespace_ptr = mono_class_get_namespace(mono_class))
    class_name = std::string(class_namespace_ptr) + "." + class_name;

  if (MonoType* class_type = mono_class_get_type(mono_class);
      mono_type_get_type(class_type) == MONO_TYPE_SZARRAY ||
      mono_type_get_type(class_type) == MONO_TYPE_ARRAY) {
    // TODO 配列型の添え字演算子を削除する
  }
  return class_name;
}

VariantType MonoScriptUtilities::GetVariantTypeFromMonoType(
    MonoType* monoType, PropertyHint* hint, std::string* hint_name) {
  PropertyHint hint_type_result = PropertyHint::kNone;
  std::string hint_name_result;
  VariantType result = VariantType::kNil;
  int32_t typeEncoding = mono_type_get_type(monoType);
  MonoClass* typeClass = mono_type_get_class(monoType);
  auto GetClass = [](const std::string& name) {
    return mono_class_from_name(
        CSharpScriptEngine::GetInstance()->GetCoreImage(),
        "BaseEngine_ScriptCore", name.c_str());
  };
  auto GetSystemClass = [](const char* nameSpace, const char* name) {
    return mono_class_from_name(mono_get_corlib(), nameSpace, name);
  };
  switch (typeEncoding) {
    case MONO_TYPE_VOID:
      result = VariantType::kVoid;
      break;
    case MONO_TYPE_BOOLEAN:
      result = VariantType::kBool;
      break;
    case MONO_TYPE_CHAR:
      result = VariantType::kUInt16;
      break;
    case MONO_TYPE_I1:
      result = VariantType::kInt8;
      break;
    case MONO_TYPE_I2:
      result = VariantType::kInt16;
      break;
    case MONO_TYPE_I4:
      result = VariantType::kInt32;
      break;
    case MONO_TYPE_I8:
      result = VariantType::kInt64;
      break;
    case MONO_TYPE_U1:
      result = VariantType::kUInt8;
      break;
    case MONO_TYPE_U2:
      result = VariantType::kUInt16;
      break;
    case MONO_TYPE_U4:
      result = VariantType::kUInt32;
      break;
    case MONO_TYPE_U8:
      result = VariantType::kUInt64;
      break;
    case MONO_TYPE_R4:
      result = VariantType::kFloat;
      break;
    case MONO_TYPE_R8:
      result = VariantType::kDouble;
      break;
    case MONO_TYPE_STRING:
      result = VariantType::kString;
      break;
    case MONO_TYPE_VALUETYPE: {
      if (typeClass == GetClass("Vector3F")) result = VariantType::kVECTOR3F;
      if (typeClass == GetClass("Vector2F")) result = VariantType::kVECTOR2F;
      break;
    }
    case MONO_TYPE_CLASS: {
      if (typeClass == GetClass("Prefab")) {
        hint_type_result = PropertyHint::kAsset;
        hint_name_result = "Prefab";
        result = VariantType::kAssetHandle;
      }
      if (typeClass == GetClass("AssetHandle"))
        result = VariantType::kAssetHandle;
    }
    break;
    case MONO_TYPE_GENERICINST: {
      MonoClass* p = mono_class_from_name(
          mono_get_corlib(), "System.Collections.Generic", "List`1");
      auto na = mono_type_get_name(monoType);
      if (typeClass == GetSystemClass("System.Collections.Generic", "List`1"))
        result = VariantType::kVoid;
    } break;
  }
  if (hint) {
    *hint = hint_type_result;
  }
  if (hint_name) {
    *hint_name = hint_name_result;
  }
  return result;
}

MonoObject* MonoScriptUtilities::GetFieldValueObject(
    MonoObject* class_instance, const std::string_view field_name,
    const bool is_property) {
  MonoClass* object_class = mono_object_get_class(class_instance);

  MonoObject* value_object = nullptr;

  if (is_property) {
    MonoProperty* class_property =
        mono_class_get_property_from_name(object_class, field_name.data());
    value_object = mono_property_get_value(class_property, class_instance,
                                           nullptr, nullptr);
  } else {
    MonoClassField* class_field =
        mono_class_get_field_from_name(object_class, field_name.data());
    value_object = mono_field_get_value_object(mono_domain_get(), class_field,
                                               class_instance);
  }

  return value_object;
}

void MonoScriptUtilities::SetFieldVariant(MonoObject* class_instance,
                                          MonoFieldInfo* field_info,
                                          const Variant& data) {
  MonoClass* object_class = mono_object_get_class(class_instance);
  MonoClassField* class_field = mono_class_get_field_from_name(
      object_class, field_info->field_info.name.c_str());

  const auto field_instance = mono_field_get_value_object(
      CSharpScriptEngine::GetInstance()->GetCoreDomain(), class_field,
      class_instance);

  if (!field_instance) return;
  const std::string name =
      mono_class_get_name(mono_object_get_class(field_instance));
  if (name == "Prefab") {
    data.Visit([class_instance, class_field](auto value) {
      void* field_data = CSharpScriptEngine::GetInstance()->CreateManagedObject(
          "BaseEngine_ScriptCore.Prefab", value);
      mono_field_set_value(class_instance, class_field, field_data);
    });
  } else {
    data.Visit([class_instance, class_field](auto value) {
      void* field_data = reinterpret_cast<void*>(&value);
      mono_field_set_value(class_instance, class_field, field_data);
    });
  }
}

Variant MonoScriptUtilities::GetFieldVariant(MonoObject* class_instance,
                                             MonoFieldInfo* field_info,
                                             bool is_property) {
  MonoClass* object_class = mono_object_get_class(class_instance);

  Variant value_object;

  if (is_property) {
    MonoProperty* class_property = mono_class_get_property_from_name(
        object_class, field_info->field_info.name.data());
    MonoObject* obj = mono_property_get_value(class_property, class_instance,
                                              nullptr, nullptr);
  } else {
    MonoClassField* class_field = mono_class_get_field_from_name(
        object_class, field_info->field_info.name.data());

    MonoObject* obj = mono_field_get_value_object(
        CSharpScriptEngine::GetInstance()->GetCoreDomain(), class_field,
        class_instance);

    const Variant variant{obj, field_info->field_info.type};
    return variant;
  }

  return value_object;
}
}  // namespace base_engine

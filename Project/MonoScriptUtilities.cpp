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
    MonoType* monoType) {
  int32_t typeEncoding = mono_type_get_type(monoType);
  MonoClass* typeClass = mono_type_get_class(monoType);
  auto GetClass = [](const std::string& name) {
    return mono_class_from_name(
        CSharpScriptEngine::GetInstance()->GetCoreImage(),
        "BaseEngine_ScriptCore", name.c_str());
  };
  switch (typeEncoding) {
    case MONO_TYPE_VOID:
      return VariantType::kVoid;
    case MONO_TYPE_BOOLEAN:
      return VariantType::kBool;
    case MONO_TYPE_CHAR:
      return VariantType::kUInt16;
    case MONO_TYPE_I1:
      return VariantType::kInt8;
    case MONO_TYPE_I2:
      return VariantType::kInt16;
    case MONO_TYPE_I4:
      return VariantType::kInt32;
    case MONO_TYPE_I8:
      return VariantType::kInt64;
    case MONO_TYPE_U1:
      return VariantType::kUInt8;
    case MONO_TYPE_U2:
      return VariantType::kUInt16;
    case MONO_TYPE_U4:
      return VariantType::kUInt32;
    case MONO_TYPE_U8:
      return VariantType::kUInt64;
    case MONO_TYPE_R4:
      return VariantType::kFloat;
    case MONO_TYPE_R8:
      return VariantType::kDouble;
    case MONO_TYPE_STRING:
      return VariantType::kString;
    case MONO_TYPE_VALUETYPE: {
      if (typeClass == GetClass("Vector3F")) return VariantType::VECTOR3F;
      if (typeClass == GetClass("Vector2F")) return VariantType::VECTOR2F;
    }
    case MONO_TYPE_CLASS: {
      if (typeClass == GetClass("Prefab")) return VariantType::kAssetHandle;
      if (typeClass == GetClass("AssetHandle"))
        return VariantType::kAssetHandle;
    }
  }
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
  const std::string name = mono_class_get_name(mono_object_get_class(field_instance));
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

    MonoObject* obj = mono_field_get_value_object( CSharpScriptEngine::GetInstance()->GetCoreDomain(),
                                                  class_field, class_instance);

    const Variant variant{obj, field_info->field_info.type};
    return variant;
  }

  return value_object;
}
}  // namespace base_engine

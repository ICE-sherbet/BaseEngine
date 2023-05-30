// @MonoScriptUtilities.h
// @brief
// @author ICE
// @date 2023/04/11
//
// @details

#pragma once
#include <mono/metadata/object.h>

#include <string>

#include "MonoClassTypeInfo.h"
#include "ScriptTypes.h"

namespace base_engine {
class MonoScriptUtilities {
 public:

  MonoScriptUtilities() = delete;

  static std::string ResolveMonoClassName(MonoClass* mono_class);
  static VariantType GetVariantTypeFromMonoType(MonoType* monoType);
  static MonoObject* GetFieldValueObject(MonoObject* class_instance,
                                         std::string_view field_name,
                                         bool is_property);
  static Variant GetFieldVariant(MonoObject* class_instance,
                                         MonoFieldInfo* field_info,
                                         bool is_property);
  static void SetFieldVariant(MonoObject* class_instance,
                              MonoFieldInfo* field_info, const Variant& data);
  template <typename TValueType>
  static TValueType Unbox(MonoObject* obj) {
    return *static_cast<TValueType*>(mono_object_unbox(obj));
  }
};
}  // namespace base_engine
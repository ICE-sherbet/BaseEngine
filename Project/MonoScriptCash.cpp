#include "MonoScriptCash.h"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/class.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/tokentype.h>

#include <functional>

#include "AssetManager.h"
#include "CSharpScriptEngine.h"
#include "MonoScriptUtilities.h"
#include "ObjectEntity.h"
#include "Script.h"
namespace base_engine {
void MonoScriptCacheStorage::BuildClassMetadata(
    Ref<AssemblyInfo>& assembly_info, MonoClass* mono_class) const {
  const std::string full_name =
      MonoScriptUtilities::ResolveMonoClassName(mono_class);

  std::hash<std::string> Hash{};
  const uint32_t class_id = Hash(full_name);
  auto& [ID, full_name_, fields_, methods_, size_, parent_id, mono_class_] =
      cache_->classes[class_id];
  full_name_ = full_name;
  ID = class_id;
  mono_class_ = mono_class;

  // TODO abstructなどの判別
  uint32_t classFlags = mono_class_get_flags(mono_class);

  MonoClass* parent_class = mono_class_get_parent(mono_class);

  if (parent_class != nullptr &&
      parent_class != GetManagedClassByName("System.Object")->mono_class) {
    const std::string parent_name =
        MonoScriptUtilities::ResolveMonoClassName(parent_class);
    parent_id = Hash(parent_name);
  }
  assembly_info->Classes.push_back(ID);
}

bool MonoScriptCacheStorage::Init() {
  cache_ = std::make_unique<Cache>();
  CacheCoreClasses();
  return true;
}

void MonoScriptCacheStorage::GenerateCacheForAssembly(
    Ref<AssemblyInfo> assembly_info) {
  const MonoTableInfo* tableInfo = mono_image_get_table_info(
      assembly_info->AssemblyImage, MONO_TABLE_TYPEDEF);
  const int32_t table_row_count = mono_table_info_get_rows(tableInfo);
  for (int32_t i = 1; i < table_row_count; i++) {
    MonoClass* monoClass = mono_class_get(assembly_info->AssemblyImage,
                                          (i + 1) | MONO_TOKEN_TYPE_DEF);
    BuildClassMetadata(assembly_info, monoClass);
  }

  for (auto class_id : assembly_info->Classes) {
    MonoClassTypeInfo& managed_class = cache_->classes.at(class_id);

    CacheClassMethods(assembly_info, managed_class);
    CacheClassFields(assembly_info, managed_class);

    if (mono_class_is_subclass_of(
            managed_class.mono_class,
            GetManagedClassByName("BaseEngine_ScriptCore.Entity")->mono_class,
            false)) {
      const auto id = std::hash<std::string>{}(managed_class.full_name);
      const AssetHandle handle =
          AssetManager::CreateMemoryOnlyAssetWithHandle<Script>(id, class_id);
      AssetManager::GetMutableMetadata(handle).file_path =
          managed_class.full_name;
    }
  }
}

MonoClassTypeInfo* MonoScriptCacheStorage::GetManagedClassByName(
    const std::string& class_name) const {
  const std::hash<std::string> hash_func{};
  return GetManagedClassById(hash_func(class_name));
}

MonoClassTypeInfo* MonoScriptCacheStorage::GetManagedClassById(
    const uint32_t class_id) const {
  if (cache_ == nullptr) return nullptr;

  if (!cache_->classes.contains(class_id)) return nullptr;

  return &cache_->classes[class_id];
}

MonoClassTypeInfo* MonoScriptCacheStorage::GetMonoObjectClass(
    MonoObject* mono_object) const {
  if (cache_ == nullptr) return nullptr;

  MonoClass* object_class = mono_object_get_class(mono_object);
  if (object_class == nullptr) return nullptr;

  return GetManagedClassByName(
      MonoScriptUtilities::ResolveMonoClassName(object_class));
}

MonoFieldInfo* MonoScriptCacheStorage::GetFieldById(
    const uint32_t field_id) const {
  if (cache_ == nullptr) return nullptr;

  if (!cache_->fields.contains(field_id)) return nullptr;

  return &cache_->fields[field_id];
}

MonoMethodInfo* MonoScriptCacheStorage::GetSpecificManagedMethod(
    const MonoClassTypeInfo* managed_class, const std::string& name,
    const uint32_t parameter_count, const bool ignore_parent) {
  if (cache_ == nullptr) return nullptr;

  if (managed_class == nullptr) return nullptr;

  MonoMethodInfo* method = nullptr;

  const std::hash<std::string> hash_func{};
  if (const uint32_t method_id =
          hash_func(managed_class->full_name + ":" + name);
      cache_->methods.contains(method_id)) {
    auto& mono_method_info = cache_->methods.at(method_id);

    if (mono_method_info.parameter_count == parameter_count) {
      method = &mono_method_info;
    }
  }

  if (method == nullptr && !ignore_parent && managed_class->parent_id != 0)
    method = GetSpecificManagedMethod(
        &cache_->classes.at(managed_class->parent_id), name, parameter_count);

  if (method == nullptr) {
    // TODO [ログ]
    // メソッド名と引数の数が一致するメソッドが存在しないことを伝える
  }

  return method;
}

void MonoScriptCacheStorage::CacheClass(std::string_view class_name,
                                        MonoClass* mono_class) {
  MonoType* classType = mono_class_get_type(mono_class);

  const std::hash<std::string> hash_func{};
  const auto id = hash_func(class_name.data());
  MonoClassTypeInfo& managed_class = cache_->classes[id];
  managed_class.full_name = class_name;

  int alignment = 0;
  managed_class.size = mono_type_size(classType, &alignment);
  managed_class.mono_class = mono_class;

  if (managed_class.full_name.find("BaseEngine_ScriptCore.") ==
      std::string::npos)
    return;

  const Ref<AssemblyInfo> core_assembly =
      CSharpScriptEngine::GetInstance()->GetCoreAssembly();
  CacheClassMethods(core_assembly, managed_class);
  CacheClassFields(core_assembly, managed_class);
}

void MonoScriptCacheStorage::CacheClassMethods(
    Ref<AssemblyInfo> assembly_info, MonoClassTypeInfo& managed_class) const {
  MonoMethod* mono_method = nullptr;
  void* method_ptr = nullptr;
  while ((mono_method = mono_class_get_methods(managed_class.mono_class,
                                               &method_ptr)) != nullptr) {
    MonoMethodSignature* sig = mono_method_signature(mono_method);

    uint32_t flags = mono_method_get_flags(mono_method, nullptr);
    char* full_name = mono_method_full_name(mono_method, FALSE);

    MonoMethodInfo method;
    std::hash<std::string> hash_func{};
    method.id = hash_func(full_name);
    method.method_info.name = full_name;
    method.parameter_count = mono_signature_get_param_count(sig);
    method.method = mono_method;
    cache_->methods[method.id] = method;
    managed_class.methods.push_back(method.id);
    mono_free(full_name);
  }
}

void MonoScriptCacheStorage::CacheClassFields(
    Ref<AssemblyInfo> assembly_info, MonoClassTypeInfo& managed_class) const {
  MonoClassField* mono_field = nullptr;
  void* field_ptr = nullptr;
  while ((mono_field = mono_class_get_fields(managed_class.mono_class,
                                             &field_ptr)) != nullptr) {
    const char* field_name = mono_field_get_name(mono_field);
    const char* full_name = mono_field_full_name(mono_field);
    const auto mono_type = mono_field_get_type(mono_field);
    const int32_t typeEncoding = mono_type_get_type(mono_type);

    MonoCustomAttrInfo* attributes =
        mono_custom_attrs_from_field(managed_class.mono_class, mono_field);

    std::hash<std::string> hash_func{};
    const auto field_id = hash_func(full_name);

    MonoFieldInfo& field = cache_->fields[field_id];
    field.id = field_id;
    field.field_info.name = field_name;
    field.field_info.type =
        MonoScriptUtilities::GetVariantTypeFromMonoType(mono_type);
    field.mono_field = mono_field;
    field.type = mono_field_get_type(mono_field);

    if (typeEncoding == MONO_TYPE_ARRAY || typeEncoding == MONO_TYPE_SZARRAY)
      field.attribute_flags |= static_cast<uint64_t>(FieldFlags::kIsArray);

    uint32_t visibility =
        mono_field_get_flags(mono_field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

    // TODO 可視性用のクラスを作る
    switch (visibility) {
      case MONO_FIELD_ATTR_PUBLIC: {
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kProtected);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPrivate);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kInternal);
        field.attribute_flags |= static_cast<uint64_t>(FieldFlags::kPublic);
        break;
      }
      case MONO_FIELD_ATTR_FAMILY: {
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPublic);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPrivate);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kInternal);
        field.attribute_flags |= static_cast<uint64_t>(FieldFlags::kProtected);
        break;
      }
      case MONO_FIELD_ATTR_ASSEMBLY: {
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPublic);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kProtected);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPrivate);
        field.attribute_flags |= static_cast<uint64_t>(FieldFlags::kInternal);
        break;
      }
      case MONO_FIELD_ATTR_PRIVATE: {
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kPublic);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kProtected);
        field.attribute_flags &= ~static_cast<uint64_t>(FieldFlags::kInternal);
        field.attribute_flags |= static_cast<uint64_t>(FieldFlags::kPrivate);
        break;
      }
    }

    {
      int align;
      field.size = mono_type_size(mono_type, &align);
    }

    if (std::ranges::find(managed_class.fields, field.id) ==
        managed_class.fields.end()) {
      managed_class.fields.push_back(field.id);
    }

    managed_class.size += field.size;
  }
}

bool MonoScriptCacheStorage::CacheCoreClasses() {
  auto CacheCoreLibClass = [this](const std::string& name,
                                  const VariantType type = VariantType::kNil) {
    const std::string full_name = "System." + name;
    CacheClass(full_name,
               mono_class_from_name(mono_get_corlib(), "System", name.c_str()));
    if (type == VariantType::MANAGED || type == VariantType::kNil) return;
    cache_->core_classes[type] = GetManagedClassByName(full_name)->id;
  };

  CacheCoreLibClass("Object");
  CacheCoreLibClass("ValueType");
  CacheCoreLibClass("Boolean");
  CacheCoreLibClass("SByte");
  CacheCoreLibClass("Int16");
  CacheCoreLibClass("Int32");
  CacheCoreLibClass("Int64");
  CacheCoreLibClass("Byte");
  CacheCoreLibClass("UInt16");
  CacheCoreLibClass("UInt32");
  CacheCoreLibClass("UInt64");
  CacheCoreLibClass("Single");
  CacheCoreLibClass("Double");
  CacheCoreLibClass("Char");
  CacheCoreLibClass("String");
  CacheClass("System.Diagnostics.StackTrace",
             mono_class_from_name(mono_get_corlib(), "System.Diagnostics",
                                  "StackTrace"));

  auto CacheBeCoreLibClass = [this](
                                 const std::string& name,
                                 const VariantType type = VariantType::kNil) {
    const std::string full_name = "BaseEngine_ScriptCore." + name;
    CacheClass(full_name, mono_class_from_name(
                              CSharpScriptEngine::GetInstance()->GetCoreImage(),
                              "BaseEngine_ScriptCore", name.c_str()));
    if (type == VariantType::MANAGED || type == VariantType::kNil) return;
    cache_->core_classes[type] = GetManagedClassByName(full_name)->id;
  };

  CacheBeCoreLibClass("Entity");

  // Math
  // Vector
  CacheBeCoreLibClass("Vector3F");
  CacheBeCoreLibClass("Vector2F");

  return true;
}
MonoScriptCacheStorage::~MonoScriptCacheStorage() = default;
}  // namespace base_engine

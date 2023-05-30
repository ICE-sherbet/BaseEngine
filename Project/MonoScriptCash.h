// @MonoScriptCash.h
// @brief
// @author ICE
// @date 2023/04/11
//
// @details

#pragma once
#include <unordered_map>
#include <vector>

#include "MonoClassTypeInfo.h"

namespace base_engine {
struct Cache {
 private:
  using MonoClassMap = std::unordered_map<MonoClassHandle, MonoClassTypeInfo>;
  using MonoMethodMap =
      std::unordered_map<MonoMethodHandle, MonoMethodInfo>;
  using MonoFieldMap =
      std::unordered_map<MonoFieldHandle, MonoFieldInfo>;
  using VariantMap = std::unordered_map<VariantType, MonoClassHandle>;

 public:
  MonoClassMap classes = {};
  MonoMethodMap methods = {};
  MonoFieldMap fields = {};
  VariantMap core_classes = {};
};

class MonoScriptCacheStorage {
 public:
  bool Init();
  void GenerateCacheForAssembly(Ref<AssemblyInfo> assembly_info);

  MonoClassTypeInfo* GetManagedClassByName(const std::string& class_name) const;
  MonoClassTypeInfo* GetManagedClassById(uint32_t class_id) const;
  MonoClassTypeInfo* GetMonoObjectClass(MonoObject* mono_object) const;
  MonoFieldInfo* GetFieldById(uint32_t field_id) const;

  MonoMethodInfo* GetSpecificManagedMethod(
      const MonoClassTypeInfo* managed_class, const std::string& name,
      uint32_t parameter_count, bool ignore_parent = false);

  ~MonoScriptCacheStorage();

 private:
  void CacheClass(std::string_view class_name, MonoClass* mono_class);
  void CacheClassMethods(Ref<AssemblyInfo> assembly_info,
                         MonoClassTypeInfo& managed_class) const;
  void CacheClassFields(Ref<AssemblyInfo> assembly_info,
                        MonoClassTypeInfo& managed_class) const;
  bool CacheCoreClasses();
  void BuildClassMetadata(Ref<AssemblyInfo>& assembly_info,
                          MonoClass* mono_class) const;
  std::unique_ptr<Cache> cache_;
};
}  // namespace base_engine

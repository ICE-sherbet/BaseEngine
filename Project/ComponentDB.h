﻿// @ComponentDB.h
// @brief
// @author ICE
// @date 2023/06/25
//
// @details

#pragma once
#include <ranges>
#include <string_view>
#include <unordered_map>

#include "Becs/SparseSet.h"
#include "MethodBind.h"
#include "ScriptTypes.h"

namespace base_engine {

struct PropertySetGet {
  VariantType property = VariantType::kNil;

  std::string setter_name;
  std::string getter_name;
  std::shared_ptr<MethodBind> setter = nullptr;
  std::shared_ptr<MethodBind> getter = nullptr;
};

class ComponentDB {
 public:
  struct ClassInfo {
    std::string name;
    uint32_t id;
    size_t size;
    std::function<void(std::shared_ptr<becs::basic_sparse_set<>>&,
                       std::allocator<becs::Entity>)>
        registry_pool_factory{};
    std::vector<PropertyInfo> properties{};
    std::unordered_map<std::string, PropertyInfo> property_map{};
    std::unordered_map<std::string, PropertySetGet> property_setget{};
    std::unordered_map<std::string, std::shared_ptr<MethodBind>> method_map{};
    std::vector<uint32_t> require_components{};

    ClassInfo(const std::string& name, uint32_t id, size_t size)
        : name(name), id(id), size(size) {}
  };

  ComponentDB() = delete;

  static void Initialize();

  template <class T>
  static void AddClass() {
    auto info =
        AddClassImpl(T::_GetClassNameStatic(), T::_GetHash(), T::_GetSize());
    info->registry_pool_factory = &T::_CreateRegistryPool;
    classes_bind_event_map_[T::_GetHash()] = &T::_Bind;
  }

  static std::shared_ptr<MethodBind> BindMethodImpl(
      std::shared_ptr<MethodBind> p_bind, const char* method_name,
      const Variant** p_defs, int p_defcount);

  template <class M, typename... VarArgs>
  static std::shared_ptr<MethodBind> BindMethod(const char* p_method_name,
                                                M p_method, VarArgs... p_args) {
    Variant args[sizeof...(p_args) + 1] = {p_args..., Variant()};
    const Variant* argptrs[sizeof...(p_args) + 1];
    for (uint32_t i = 0; i < sizeof...(p_args); i++) {
      argptrs[i] = &args[i];
    }
    std::shared_ptr<MethodBind> bind = CreateMethodBind(p_method);
    return BindMethodImpl(bind, p_method_name,
                          sizeof...(p_args) == 0
                              ? nullptr
                              : static_cast<const Variant**>(argptrs),
                          sizeof...(p_args));
  }

  static std::shared_ptr<MethodBind> GetMethod(const std::string& class_name,
                                               const std::string& method_name);

  static void AddProperty(const std::string& class_name,
                          const PropertyInfo& info,
                          const std::string& setter_name,
                          const std::string& getter_name);

  static void GetPropertyList(const std::string& class_name,
                              std::list<PropertyInfo>* list);
  static void GetPropertyList(uint32_t class_id, std::list<PropertyInfo>* list);

  /**
   * \brief 指定クラスのプロパティを取得する
   * \param object 指定クラスのインスタンス
   * \param class_name 指定クラスの名前
   * \param property_name プロパティ名
   * \param return_value プロパティのゲッターの戻り値
   * \return プロパティを取得できたかどうか
   */
  static bool TryGetProperty(void* object, const std::string& class_name,
                             const std::string& property_name,
                             Variant& return_value);
  static bool TrySetProperty(void* object, const std::string& class_name,
                             const std::string& property_name,
                             const Variant& value);

  static void AddRequireComponent(const std::string& class_name,
                                  const std::string& require_component);
  static void AddRequireComponent(uint32_t class_id,
                                  uint32_t require_class_id);
  static void GetRequireComponent(const std::string& class_name,
                                  std::list<uint32_t>* list);
  static void GetRequireComponent(uint32_t class_id, std::list<uint32_t>* list);

  static void GetClassHash(std::list<uint32_t>* list);

  static void GetClassList(std::list<uint32_t>* list);

  static std::shared_ptr<ClassInfo> GetClass(uint32_t id);

  static std::shared_ptr<ClassInfo> GetClass(const std::string& name);

 private:
  static std::shared_ptr<ClassInfo> AddClassImpl(std::string_view class_name,
                                                 uint32_t id, size_t size);

  static std::unordered_map<std::string, std::shared_ptr<ClassInfo>>
      classes_name_map_;
  static std::unordered_map<uint32_t, std::shared_ptr<ClassInfo>>
      classes_id_map_;
  static std::unordered_map<uint32_t, std::function<void()>>
      classes_bind_event_map_;
  
};
}  // namespace base_engine

// @ComponentDB.h
// @brief
// @author ICE
// @date 2023/06/25
//
// @details

#pragma once
#include <string_view>
#include <unordered_map>

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
    std::vector<PropertyInfo> properties{};
    std::unordered_map<std::string, PropertyInfo> property_map{};
    std::unordered_map<std::string, PropertySetGet> property_setget{};
    std::unordered_map<std::string, std::shared_ptr<MethodBind>> method_map{};
  };

  ComponentDB() = delete;

  template <class T>
  static void AddClass() {
    AddClassImpl(T::_GetClassNameStatic());
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

  static bool TryGetProperty(void* object, const std::string& class_name,
                                const std::string& property_name,
                                Variant& return_value);
  static bool TrySetProperty(void* object, const std::string& class_name,
																const std::string& property_name,
																const Variant& value);

 private:
  static void AddClassImpl(std::string_view class_name) {
    classes_[class_name.data()] = ClassInfo{class_name.data()};
  }

  static std::unordered_map<std::string, ClassInfo> classes_;
};
}  // namespace base_engine

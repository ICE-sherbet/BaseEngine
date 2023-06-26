#include "ComponentDB.h"

namespace base_engine {
std::unordered_map<std::string, ComponentDB::ClassInfo> ComponentDB::classes_;

std::shared_ptr<MethodBind> ComponentDB::BindMethodImpl(
    std::shared_ptr<MethodBind> p_bind, const char* method_name,
    const Variant** p_defs, int p_defcount) {
  p_bind->SetMethodName(method_name);

  const auto instance_type = p_bind->ClassName();
  ClassInfo* type = &classes_[instance_type];

  type->method_map[method_name] = p_bind;
  return p_bind;
}

std::shared_ptr<MethodBind> ComponentDB::GetMethod(
    const std::string& class_name, const std::string& method_name) {
  if (!classes_.contains(class_name)) return nullptr;
  auto& type = classes_[class_name];

  const auto method = type.method_map[method_name];

  return method;
}

void ComponentDB::AddProperty(const std::string& class_name,
                              const PropertyInfo& info,
                              const std::string& setter_name,
                              const std::string& getter_name) {
  auto& class_info = classes_[class_name.data()];
  class_info.properties.emplace_back(info);
  class_info.property_map[info.name] = info;
  auto setget = PropertySetGet{info.type, setter_name, getter_name};

  std::shared_ptr<MethodBind> setter = nullptr;
  if (!setter_name.empty()) {
    setter = GetMethod(class_name, setter_name);
  }
  std::shared_ptr<MethodBind> getter = nullptr;
  if (!getter_name.empty()) {
    getter = GetMethod(class_name, getter_name);
  }

  setget.setter = setter;
  setget.getter = getter;

  class_info.property_setget[info.name] = setget;
}

bool ComponentDB::TryGetProperty(void* object, const std::string& class_name,
                                 const std::string& property_name,
                                 Variant& return_value) {
  if (!classes_.contains(class_name)) return false;
  auto& type = classes_[class_name];

  if (!type.property_map.contains(property_name)) return false;
  auto& property = type.property_map[property_name];

  if (!type.property_setget.contains(property_name)) return false;
  auto& setget = type.property_setget[property_name];

  if (setget.getter == nullptr) return false;

  return_value = setget.getter->call(object, nullptr, 0);
  return true;
}

bool ComponentDB::TrySetProperty(void* object, const std::string& class_name,
                                 const std::string& property_name,
                                 const Variant& value) {
  if (!classes_.contains(class_name)) return false;
  auto& type = classes_[class_name];

  if (!type.property_map.contains(property_name)) return false;
  auto& property = type.property_map[property_name];

  if (!type.property_setget.contains(property_name)) return false;
  auto& setget = type.property_setget[property_name];

  if (setget.setter == nullptr) return false;

  const Variant* arg[1] = {&value};
  setget.setter->call(object, arg, 1);

  return true;
}
}  // namespace base_engine

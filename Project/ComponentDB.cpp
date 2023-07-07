#include "ComponentDB.h"

#include "BodyMask.h"
#include "DataComponents.h"
#include "ShapeComponents.h"

namespace base_engine {
using ClassInfoPtr = std::shared_ptr<ComponentDB::ClassInfo>;
std::unordered_map<std::string, ClassInfoPtr> ComponentDB::classes_name_map_;
std::unordered_map<uint32_t, ClassInfoPtr> ComponentDB::classes_id_map_;

void ComponentDB::Initialize() {
  using namespace component;
  TagComponent::_Initialize();
  SpriteRendererComponent::_Initialize();
  ScriptComponent::_Initialize();
  AudioComponent::_Initialize();
  TransformComponent::_Initialize();

  physics::BodyMask::_Initialize();
  physics::CircleShape::_Initialize();
}

std::shared_ptr<MethodBind> ComponentDB::BindMethodImpl(
    std::shared_ptr<MethodBind> p_bind, const char* method_name,
    const Variant** p_defs, int p_defcount) {
  p_bind->SetMethodName(method_name);

  const auto instance_type = p_bind->ClassName();
  const auto type = classes_name_map_[instance_type];

  type->method_map[method_name] = p_bind;
  return p_bind;
}

std::shared_ptr<MethodBind> ComponentDB::GetMethod(
    const std::string& class_name, const std::string& method_name) {
  if (!classes_name_map_.contains(class_name)) return nullptr;
  const auto& type = classes_name_map_[class_name];

  const auto method = type->method_map[method_name];

  return method;
}

void ComponentDB::AddProperty(const std::string& class_name,
                              const PropertyInfo& info,
                              const std::string& setter_name,
                              const std::string& getter_name) {
  auto& class_info = classes_name_map_[class_name.data()];
  class_info->properties.emplace_back(info);
  class_info->property_map[info.name] = info;
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

  class_info->property_setget[info.name] = setget;
}

void ComponentDB::GetPropertyList(const std::string& class_name,
                                  std::list<PropertyInfo>* list) {
  auto& class_info = classes_name_map_[class_name];
  for (const auto& property : class_info->properties) {
    list->emplace_back(property);
  }
}

void ComponentDB::GetPropertyList(uint32_t class_id,
                                  std::list<PropertyInfo>* list) {
  if (!classes_id_map_.contains(class_id)) return;
  auto& class_info = classes_id_map_[class_id];
  for (const auto& property : class_info->properties) {
    list->emplace_back(property);
  }
}

bool ComponentDB::TryGetProperty(void* object, const std::string& class_name,
                                 const std::string& property_name,
                                 Variant& return_value) {
  if (!classes_name_map_.contains(class_name)) return false;
  auto& type = classes_name_map_[class_name];

  if (!type->property_map.contains(property_name)) return false;
  auto& property = type->property_map[property_name];

  if (!type->property_setget.contains(property_name)) return false;
  auto& setget = type->property_setget[property_name];

  if (setget.getter == nullptr) return false;

  return_value = setget.getter->call(object, nullptr, 0);
  return true;
}

bool ComponentDB::TrySetProperty(void* object, const std::string& class_name,
                                 const std::string& property_name,
                                 const Variant& value) {
  if (!classes_name_map_.contains(class_name)) return false;
  auto& type = classes_name_map_[class_name];

  if (!type->property_map.contains(property_name)) return false;
  auto& property = type->property_map[property_name];

  if (!type->property_setget.contains(property_name)) return false;
  auto& setget = type->property_setget[property_name];

  if (setget.setter == nullptr) return false;

  const Variant* arg[1] = {&value};
  setget.setter->call(object, arg, 1);

  return true;
}

void ComponentDB::GetClassHash(std::list<uint32_t>* list) {
  for (const auto& clazz : classes_name_map_ | std::views::values) {
    list->emplace_back(clazz->id);
  }
}

void ComponentDB::GetClassList(std::list<uint32_t>* list) {
  for (const auto& clazz : classes_name_map_ | std::views::values) {
    list->emplace_back(clazz->id);
  }
}

std::shared_ptr<ComponentDB::ClassInfo> ComponentDB::GetClass(const uint32_t id) {
  if (!classes_id_map_.contains(id)) return nullptr;
  return classes_id_map_[id];
}

std::shared_ptr<ComponentDB::ClassInfo> ComponentDB::GetClass(
    const std::string& name) {
  if (!classes_name_map_.contains(name)) return nullptr;
  return classes_name_map_[name];
}

void ComponentDB::AddClassImpl(std::string_view class_name, uint32_t id) {
  const auto info =
      std::make_shared<ClassInfo>(ClassInfo{class_name.data(), id});
  classes_name_map_[class_name.data()] = info;
  classes_id_map_[id] = info;
}
}  // namespace base_engine

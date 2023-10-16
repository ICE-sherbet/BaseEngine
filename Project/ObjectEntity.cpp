#include "ObjectEntity.h"

#include "CSharpScriptEngine.h"

namespace base_engine {
void ObjectEntity::SetParent(const ObjectEntity parent) {
  SetParent(parent, true);
}

void ObjectEntity::SetParent(ObjectEntity parent,
                             const bool world_position_stays) {
  ObjectEntity current_parent = this->GetParent();
  if (current_parent == parent) return;

  if (current_parent) current_parent.RemoveChild(*this);

  SetParentUUID(parent.GetUUID());

  if (!parent) return;
  auto& parent_children = parent.Children();
  if (const UUID uuid = GetUUID();
      std::ranges::find(parent_children, uuid) != parent_children.end())
    return;
  parent_children.emplace_back(GetUUID());
  parent.GetComponent<component::TransformComponent>().SetChildren(
      parent_children);

  if (!world_position_stays) return;
  const auto world_transform_matrix =
      scene_->GetWorldSpaceTransformMatrix(parent);
  auto& transform = GetComponent<component::TransformComponent>();
  const auto local = transform.GetLocalTransform();
  const auto matrix =
      mat::Inverse(world_transform_matrix) * transform.GetGlobalTransform();

  transform.SetGlobalTransform(matrix);
}

void ObjectEntity::SetParentUUID(const UUID& parent) {
  GetComponent<component::HierarchyComponent>().parent_handle = parent;
  GetComponent<component::TransformComponent>().SetParent(parent);
}

void ObjectEntity::GetPropertyList(std::list<PropertyInfo>* list) {
  if (!scene_) return;
  auto&& registry = scene_->GetRegistry();

  for (const auto& storage : registry.storage()) {
    auto clazz = ComponentDB::GetClass(storage.first);
    if (!clazz) continue;
    auto data = storage.second.contains(GetHandle());
    if (!data) continue;

    ComponentDB::GetPropertyList(clazz->name, list);
  }
  if (HasComponent<component::ScriptComponent>()) {
    const auto sc = GetComponent<component::ScriptComponent>();
    const auto id =
        CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(sc);
    const auto clazz =
        CSharpScriptEngine::GetInstance()->GetManagedClassById(id);
    for (const uint32_t field : clazz->fields) {
      const auto property =
          CSharpScriptEngine::GetInstance()->GetFieldById(field)->field_info;
      list->emplace_back(property);
    }
  }
}

void ObjectEntity::GetClassPropertyList(const std::string& class_name,
                                        std::list<PropertyInfo>* list) {
  ComponentDB::GetPropertyList(class_name, list);
  if (component::ScriptComponent::_GetClassNameStatic() == class_name &&
      HasComponent<component::ScriptComponent>()) {
    const auto sc = GetComponent<component::ScriptComponent>();
    const auto id =
        CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(sc);
    const auto clazz =
        CSharpScriptEngine::GetInstance()->GetManagedClassById(id);
    for (const uint32_t field : clazz->fields) {
      const auto property =
          CSharpScriptEngine::GetInstance()->GetFieldById(field)->field_info;
      list->emplace_back(property);
    }
  }
}

void ObjectEntity::GetClassPropertyList(uint32_t class_id,
                                        std::list<PropertyInfo>* list) {
  ComponentDB::GetPropertyList(class_id, list);
  if (component::ScriptComponent::_GetHash() == class_id &&
      HasComponent<component::ScriptComponent>()) {
    for (const auto sc = GetComponent<component::ScriptComponent>();
         const uint32_t field : sc.field_ids) {
      const auto property =
          CSharpScriptEngine::GetInstance()->GetFieldById(field)->field_info;
      list->emplace_back(property);
    }
  }
}

bool ObjectEntity::TryGetProperty(const std::string& class_name,
                                  const PropertyInfo& property_info,
                                  Variant& return_value) const {
  if (HasComponent<component::ScriptComponent>() &&
      class_name == component::ScriptComponent::_GetClassNameStatic()) {
    if (auto field = CSharpScriptEngine::GetInstance()->GetFieldStorage(
            *this, property_info.id)) {
      return_value = field->GetValueVariant();
      return true;
    }
  }
  auto& registry = scene_->GetRegistry();
  const auto class_id = ComponentDB::GetClass(class_name)->id;
  for (const auto& [id, storage] : registry.storage()) {
    if (class_id != id) continue;

    const auto data = storage.try_get(GetHandle());
    if (!data) continue;

    return ComponentDB::TryGetProperty(data, class_name, property_info.name,
                                       return_value);
  }
  return false;
}

bool ObjectEntity::TryGetProperty(const std::string& class_name,
                                  const std::string& property_name,
                                  Variant& return_value) const {
  if (!scene_) return false;
  if (HasComponent<component::ScriptComponent>() &&
      class_name == component::ScriptComponent::_GetClassNameStatic() && property_name != "script") {
    const auto class_id =
        CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(
            GetComponent<component::ScriptComponent>());
    if (class_id == 0) return false;
    const auto managed_class =
        CSharpScriptEngine::GetInstance()->GetManagedClassById(class_id);
    if (!managed_class) return false;
    for (const uint32_t value : managed_class->fields) {
      if (auto field =
              CSharpScriptEngine::GetInstance()->GetFieldStorage(*this, value);
          field && field->GetFieldInfo()->field_info.name == property_name) {
        return_value = field->GetValueVariant();
        return true;
      }
    }
  }
  auto& registry = scene_->GetRegistry();
  const auto class_id = ComponentDB::GetClass(class_name)->id;
  for (const auto& [id, storage] : registry.storage()) {
    if (class_id != id) continue;

    const auto data = storage.try_get(GetHandle());
    if (!data) continue;

    return ComponentDB::TryGetProperty(data, class_name, property_name,
                                       return_value);
  }
  return false;
}

bool ObjectEntity::TrySetProperty(const std::string& class_name,
                                  const PropertyInfo& property_info,
                                  const Variant& value) {
  if (HasComponent<component::ScriptComponent>() &&
      class_name == component::ScriptComponent::_GetClassNameStatic()) {
    if (auto field = CSharpScriptEngine::GetInstance()->GetFieldStorage(
            *this, property_info.id)) {
      field->SetValueVariant(value);
      return true;
    }
  }
  auto& registry = scene_->GetRegistry();
  auto clazz = ComponentDB::GetClass(class_name);
  const auto class_id = clazz->id;

  if (!registry.valid(class_id)) {
    registry.create_pool(class_id, clazz->registry_pool_factory);
  }
  for (const auto& [id, storage] : registry.storage()) {
    if (class_id != id) continue;
    if (!storage.contains(GetHandle())) storage.try_emplace(GetHandle(), false);
    const auto data = storage.try_get(GetHandle());
    if (!data) continue;

    return ComponentDB::TrySetProperty(data, class_name, property_info.name,
                                       value);
  }
  return false;
}

bool ObjectEntity::TrySetProperty(const std::string& class_name,
                                  const std::string& property_name,
                                  const Variant& value) {
  if (HasComponent<component::ScriptComponent>() &&
      class_name == component::ScriptComponent::_GetClassNameStatic()) {

    if (property_name == "script")
    {
      auto& registry = scene_->GetRegistry();

      const auto data =
          registry.storage(component::ScriptComponent::_GetHash())
              ->try_get(GetHandle());
      bool result =
          ComponentDB::TrySetProperty(data, class_name, property_name, value);
      CSharpScriptEngine::GetInstance()->InitializeScriptEntity(*this);
      return result;
    }
    const auto class_id =
        CSharpScriptEngine::GetInstance()->GetScriptClassIdFromComponent(
            GetComponent<component::ScriptComponent>());
    if (class_id == 0) {
      return false;
    }
    const auto managed_class =
        CSharpScriptEngine::GetInstance()->GetManagedClassById(class_id);
    if (!managed_class) return false;
    for (const uint32_t field : managed_class->fields) {
      auto storage =
          CSharpScriptEngine::GetInstance()->GetFieldStorage(*this, field);
      if (storage &&
          storage->GetFieldInfo()->field_info.name == property_name) {
        storage->SetValueVariant(value);
        return true;
      }
    }
  }
  auto& registry = scene_->GetRegistry();
  auto clazz = ComponentDB::GetClass(class_name);
  const auto class_id = clazz->id;

  if (!registry.valid(class_id)) {
    registry.create_pool(class_id, clazz->registry_pool_factory);
  }
  for (const auto& [id, storage] : registry.storage()) {
    if (class_id != id) continue;

    const auto data = storage.try_get(GetHandle());
    if (!data) continue;

    return ComponentDB::TrySetProperty(data, class_name, property_name, value);
  }
  return false;
}
}  // namespace base_engine

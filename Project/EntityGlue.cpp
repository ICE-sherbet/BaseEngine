#include "EntityGlue.h"

namespace base_engine::glue::internal_calls {
bool EntityHasComponent(uint64_t entity_id,
                        MonoReflectionType* reflection_type) {
  auto entity = GetEntity(entity_id);
  if (!entity) {
    return false;
  }

  if (reflection_type == nullptr) {
    return false;
  }

  MonoType* managedType = mono_reflection_type_get_type(reflection_type);
  if (!ManagedComponentStorage::GetInstance()->Contains(managedType)) {
    char* typeName = mono_type_get_name(managedType);
    mono_free(typeName);
    return false;
  }

  return ManagedComponentStorage::GetInstance()->HasComponent(managedType,
                                                              entity);
}

void EntityAddComponent(const uint64_t entity_id,
                        MonoReflectionType* reflection_type) {
  auto entity = GetEntity(entity_id);
  if (!entity) {
    return;
  }

  if (reflection_type == nullptr) {
    return;
  }

  MonoType* managed_type = mono_reflection_type_get_type(reflection_type);

  if (!ManagedComponentStorage::GetInstance()->Contains(managed_type)) {
    char* type_name = mono_type_get_name(managed_type);
    mono_free(type_name);
    return;
  }

  ManagedComponentStorage::GetInstance()->CreateComponent(managed_type, entity);
}

uint64_t EntityGetChild(const uint64_t entity_id, int index) {
  const auto entity = GetEntity(entity_id);
  if (!entity) {
    return 0;
  }
  const auto& children = entity.Children();

  if (index >= children.size()) {
    return 0;
  }
  if (index < 0) {
    index = static_cast<int>(children.size()) + index;
    if (index < 0) return 0;
  }
  return children[index];
}

void EntitySetParent(const uint64_t entity_id, const uint64_t parent_id,
                     bool world_position_stays) {
  auto entity = GetEntity(entity_id);
  if (!entity) {
    return;
  }

  if (parent_id != 0) {
    const auto parent = GetEntity(parent_id);
    if (!parent)
    {
      return;
    }
    entity.SetParent(parent, world_position_stays);
  } else {
    Ref<Scene> scene = CSharpScriptEngine::GetInstance()->GetScene();
    scene->UnParentEntity(entity, world_position_stays);
  }
}
}  // namespace base_engine::glue::internal_calls

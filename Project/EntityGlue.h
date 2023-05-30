// @EntityGlue.h
// @brief
// @author ICE
// @date 2023/04/17
//
// @details

#pragma once
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "CSharpScriptEngine.h"
#include "ManagedComponentStorage.h"
#include "MonoGlueMacro.h"
#include "ObjectEntity.h"
#include "Scene.h"

namespace base_engine::glue::internal_calls {
static inline ObjectEntity GetEntity(const uint64_t entity_id) {
  Ref<Scene> scene = CSharpScriptEngine::GetInstance()->GetScene();
  return scene->TryGetEntityWithUUID(entity_id);
};

bool EntityHasComponent(uint64_t entity_id,
                        MonoReflectionType* reflection_type);

void EntityAddComponent(uint64_t entity_id,
                        MonoReflectionType* reflection_type);

uint64_t EntityGetChild(uint64_t entity_id, int index);

void EntitySetParent(uint64_t entity_id, uint64_t parent_id,
                     bool world_position_stays);

struct EntityGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(EntityHasComponent);
    BE_ADD_INTERNAL_CALL(EntityAddComponent);
    BE_ADD_INTERNAL_CALL(EntityGetChild);
    BE_ADD_INTERNAL_CALL(EntitySetParent);
  }
};
}  // namespace base_engine::glue::internal_calls
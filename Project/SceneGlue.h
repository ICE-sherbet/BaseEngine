// @SceneGlue.h
// @brief
// @author ICE
// @date 2023/04/19
//
// @details

#pragma once

#include "MonoGlueMacro.h"
#include "ObjectEntity.h"
#include "ScriptTypes.h"
namespace base_engine::glue::internal_calls {
uint64_t SceneCreateEntity(MonoString* tag);
uint64_t SceneCreateEntityByScript(MonoString* tag,MonoReflectionType* reflection_type);
uint64_t SceneFindEntityByTag(MonoString* tag);
void SceneDestroyEntity(uint64_t id);
struct SceneGlue {
  void operator()() const {
    BE_ADD_INTERNAL_CALL(SceneCreateEntity);
    BE_ADD_INTERNAL_CALL(SceneCreateEntityByScript);
    BE_ADD_INTERNAL_CALL(SceneFindEntityByTag);
    BE_ADD_INTERNAL_CALL(SceneDestroyEntity);
  }
};
}  // namespace base_engine::glue::internal_calls
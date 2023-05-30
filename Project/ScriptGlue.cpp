#include "ScriptGlue.h"

#include "EntityGlue.h"

namespace base_engine::glue::internal_calls {
MonoObject* ScriptGetInstance(const uint64_t id) {
  const auto entity = GetEntity(id);

  const auto& component = entity.GetComponent<component::ScriptComponent>();

  if (!CSharpScriptEngine::GetInstance()->IsScriptValid(
          component.script_class_handle)) {
    return nullptr;
  }

  return MonoGCManager::GetReferencedObject(component.managed_instance);
}
}  // namespace base_engine::glue::internal_calls

#include "PhysicsContactListenerSystem.h"

#include "CSharpScriptEngine.h"
#include "OnCollisionTag.h"

namespace base_engine::physics {
void PhysicsContactListenerSystem::OnUpdate() {
  auto view =
      GetScene()
          ->GetAllEntitiesWith<component::ScriptComponent,
                               component::IdComponent, OnCollisionEnterTag>();

  auto ids_view = GetScene()->GetAllEntitiesWith<component::IdComponent>();
  GetScene()->GetRegistry();
  for (const auto entity : view) {
    const auto& [script, id, tag] = view.get(entity);
    const auto& [id_target] = ids_view.get(tag.target);
    CSharpScriptEngine::CallMethod(script.managed_instance,
                                   "OnCollisionEnterInternal", id_target.uuid);
    view.storage<OnCollisionEnterTag>().remove(entity);
  }
}
}  // namespace base_engine::physics

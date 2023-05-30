#include "PhysicsContactListenerSystem.h"

#include "CSharpScriptEngine.h"
#include "OnCollisionTag.h"

namespace base_engine::physics {
void PhysicsContactListenerSystem::OnUpdate() {
  auto view = GetScene()
                  ->GetAllEntitiesWith<component::ScriptComponent,
                                       OnCollisionEnterTag>();
  GetScene()->GetRegistry();
  for (const auto entity : view) {
    const auto& [script, tag] = view.get(entity);
    CSharpScriptEngine::CallMethod(script.managed_instance,
                                   "OnCollisionEnterInternal", entity);
    view.storage<OnCollisionEnterTag>().remove(entity);
  }
}
}  // namespace base_engine::physics

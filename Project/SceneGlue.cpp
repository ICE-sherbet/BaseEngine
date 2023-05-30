#include "SceneGlue.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "AssetManager.h"
#include "CSharpScriptEngine.h"
#include "ManagedComponentStorage.h"
#include "MonoScriptUtilities.h"
#include "ObjectEntity.h"
#include "Script.h"

namespace base_engine::glue::internal_calls {
uint64_t SceneCreateEntity(MonoString* tag) {
  auto scene = CSharpScriptEngine::GetInstance()->GetScene();
  BE_CORE_ASSERT(scene, "No active Scene");
  const std::string name = mono_string_to_utf8(tag);
  return scene->CreateEntity(name).GetUUID();
}

uint64_t SceneCreateEntityByScript(MonoString* tag,
                           MonoReflectionType* reflection_type) {
	const auto script_engine = CSharpScriptEngine::GetInstance();
  auto scene = script_engine->GetScene();
  BE_CORE_ASSERT(scene, "No active Scene");
  const std::string name = mono_string_to_utf8(tag);

  if (reflection_type == nullptr) {
    BE_CONSOLE_LOG_ERROR(
        "Scene.CreateEntity - スクリプトが null "
        "なのでエンティティを生成出来ません。");
    return 0;
  }

  MonoType* managed_type = mono_reflection_type_get_type(reflection_type);
  const auto info = script_engine->GetManagedClassByName(
      MonoScriptUtilities::ResolveMonoClassName(
          mono_type_get_class(managed_type)));
  const auto entity = scene->CreateEntity();

  auto& script = entity.AddComponent<component::ScriptComponent>();
  auto asset = AssetManager::GetAsset<Script>(info->id);
  script.script_class_handle = asset->handle_;

  CSharpScriptEngine::GetInstance()->RuntimeInitializeScriptEntity(entity);
  return entity.GetUUID();
}

uint64_t SceneFindEntityByTag(MonoString* tag) {
  auto scene = CSharpScriptEngine::GetInstance()->GetScene();
  BE_CORE_ASSERT(scene, "No active Scene");
  const std::string name = mono_string_to_utf8(tag);
  return scene->FindEntityWithTag(name).GetUUID();
}

void SceneDestroyEntity(const uint64_t id) {
  auto scene = CSharpScriptEngine::GetInstance()->GetScene();
  BE_CORE_ASSERT(scene, "No active Scene");
  scene->DestroyEntity(id);
}
}  // namespace base_engine::glue::internal_calls

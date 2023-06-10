#include "CSharpScriptEngine.h"

#include <source_location>

#pragma comment(lib, "mono-2.0-sgen.lib")
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>

#include <fstream>
#include <iostream>
#include <ranges>

#include "AssetManager.h"
#include "MonoGCManager.h"
#include "MonoGlue.h"
#include "MonoScriptCash.h"
#include "ObjectEntity.h"
#include "Script.h"

constexpr const char* kCoreMonoAssemblyPath = "BaseEngine_ScriptCore.dll";

void TestMethod() {}
namespace base_engine {
CSharpScriptEngine* CSharpScriptEngine::instance_ = nullptr;

template <auto Func>
consteval std::string_view GetFunctionName() {
  constexpr std::source_location location = std::source_location::current();
  constexpr std::string_view method_type = location.function_name();
  constexpr std::size_t begin = method_type.find("GetFunctionName") + 15;
  constexpr std::size_t end = method_type.find(">(void)") - 1;
  constexpr std::string_view name = method_type.substr(begin, end - begin);
  constexpr std::size_t begin_method = name.find_last_of(' ') + 1;
  constexpr std::size_t end_method = name.find_last_of('(');

  return name.substr(begin_method, end_method - begin_method);
}

template <auto F>
void AddInternalCall() {
  const std::string cs_method_name =
      std::string{"BaseEngine_ScriptCore.InternalCalls::"}.append(
          GetFunctionName<F>());

  mono_add_internal_call(cs_method_name.c_str(), static_cast<void*>(F));
}

void CSharpScriptEngine::LoadMonoAssembly() {
  MonoImageOpenStatus status;

  mono_state_.core_assembly_info_ = Ref<AssemblyInfo>::Create();

  mono_state_.core_assembly_info_->AssemblyImage =
      mono_image_open_full(kCoreMonoAssemblyPath, &status, 0);

  mono_state_.core_assembly_info_->Assembly = mono_assembly_load_from_full(
      mono_state_.core_assembly_info_->AssemblyImage, kCoreMonoAssemblyPath,
      &status, 0);
}

void CSharpScriptEngine::InitializeRuntime() {
  for (const auto view = scene_state_.scene_context->GetAllEntitiesWith<
                         component::IdComponent, component::ScriptComponent>();
       const auto handle : view) {
    const auto entity = scene_state_.scene_context->GetEntityWithUUID(
        view.get<component::IdComponent>(handle).uuid);
    RuntimeInitializeScriptEntity(entity);
  }
}

void CSharpScriptEngine::ShutdownRuntime() {
  for (const auto& id : scene_state_.script_instances | std::views::keys) {
    const auto entity = scene_state_.scene_context->TryGetEntityWithUUID(id);

    if (!entity) continue;
    ShutdownRuntimeScriptEntity(entity);
  }

  scene_state_.script_instances.clear();
  MonoGCManager::CollectGarbage(false);
  while (!scene_state_.runtime_duplicated_script_entities.empty())
    scene_state_.runtime_duplicated_script_entities.pop();
}

void CSharpScriptEngine::ShutdownRuntimeScriptEntity(ObjectEntity entity) {
  if (!entity.HasComponent<component::ScriptComponent>()) return;

  auto&& script_component = entity.GetComponent<component::ScriptComponent>();

  for (auto field_id : script_component.field_ids) {
    Ref<IFieldStorage> field_storage =
        scene_state_.field_map[entity.GetUUID()][field_id];
    if (field_storage) field_storage->SetRuntimeInstance(nullptr);
  }
  MonoGCManager::ReleaseObjectReference(script_component.managed_instance);
  script_component.managed_instance = nullptr;
  script_component.is_runtime_initialized = false;
}

void CSharpScriptEngine::InitializeScriptEntity(ObjectEntity entity) {
  if (!entity.HasComponent<component::ScriptComponent>()) return;

  const auto entity_id = entity.GetUUID();
  const auto scene_id = entity.GetSceneUUID();

  if (scene_state_.script_instances.contains(entity_id)) return;

  auto& sc = entity.GetComponent<component::ScriptComponent>();

  const auto class_info =
      GetManagedClassById(GetScriptClassIdFromComponent(sc));
  if (!class_info) return;

  sc.field_ids.clear();

  for (auto field_id : class_info->fields) {
    MonoFieldInfo* field_info = GetFieldById(field_id);
    if (field_info == nullptr) continue;
    scene_state_.field_map[entity_id][field_id] =
        Ref<FieldStorage>::Create(field_info);

    sc.field_ids.emplace_back(field_id);
  }

  if (!scene_state_.script_entities.contains(scene_id)) {
    scene_state_.script_entities[scene_id] = std::vector<UUID>();
  }
  scene_state_.script_entities[scene_id].emplace_back(entity_id);
}

void CSharpScriptEngine::RuntimeInitializeScriptEntity(ObjectEntity entity) {
  const auto script_component =
      entity.GetComponent<component::ScriptComponent>();

  if (script_component.is_runtime_initialized) return;

  if (!IsScriptValid(script_component.script_class_handle)) {
    // TODO [ログ][エラー処理] エラーログ出力
    return;
  }
  const auto uuid = entity.GetUUID();
  MonoObject* runtime_instance = CreateManagedObject(
      GetScriptClassIdFromComponent(script_component), uuid);
  const GCHandle instance_handle =
      MonoGCManager::CreateObjectReference(runtime_instance, false);
  entity.GetComponent<component::ScriptComponent>().managed_instance =
      instance_handle;
  scene_state_.script_instances[entity.GetUUID()] = instance_handle;
  for (auto field_id : script_component.field_ids) {
    if (UUID entity_id = entity.GetUUID();
        scene_state_.field_map.contains(entity_id))
      scene_state_.field_map[entity_id][field_id]->SetRuntimeInstance(
          instance_handle);
  }

  CallMethod(instance_handle, "OnCreate");
  entity.GetComponent<component::ScriptComponent>().is_runtime_initialized =
      true;
}

void CSharpScriptEngine::DuplicateScriptInstance(ObjectEntity entity,
                                                 ObjectEntity target_entity) {
  if (!entity.HasComponent<component::ScriptComponent>() ||
      !target_entity.HasComponent<component::ScriptComponent>())
    return;

  const auto& src_script_comp =
      entity.GetComponent<component::ScriptComponent>();
  auto& dst_script_comp =
      target_entity.GetComponent<component::ScriptComponent>();

  if (src_script_comp.script_class_handle !=
      dst_script_comp.script_class_handle) {
    const auto src_class = storage_->GetManagedClassById(
        GetScriptClassIdFromComponent(src_script_comp));
    const auto dst_class = storage_->GetManagedClassById(
        GetScriptClassIdFromComponent(dst_script_comp));
    return;
  }

  ShutdownScriptEntity(target_entity);
  InitializeScriptEntity(target_entity);

  const UUID target_entity_id = target_entity.GetUUID();
  const UUID src_entity_id = entity.GetUUID();

  for (auto field_id : src_script_comp.field_ids) {
    if (!scene_state_.field_map.contains(src_entity_id)) break;

    if (!scene_state_.field_map.at(src_entity_id).contains(field_id)) continue;

    scene_state_.field_map[target_entity_id][field_id]->SetValueVariant(
        scene_state_.field_map[src_entity_id][field_id]->GetValueVariant());
  }
  if (scene_state_.scene_context && scene_state_.scene_context->IsPlaying()) {
    // scene_state_.runtime_duplicated_script_entities.push(target_entity);
    RuntimeInitializeScriptEntity(target_entity);
  }
}

void CSharpScriptEngine::InitializeRuntimeDuplicatedEntities() {
  while (!scene_state_.runtime_duplicated_script_entities.empty()) {
    const auto& entity = scene_state_.runtime_duplicated_script_entities.top();

    if (!entity) {
      scene_state_.runtime_duplicated_script_entities.pop();
      continue;
    }

    RuntimeInitializeScriptEntity(entity);
    scene_state_.runtime_duplicated_script_entities.pop();
  }
}

void CSharpScriptEngine::ShutdownScriptEntity(ObjectEntity entity, bool erase) {
  if (!entity.HasComponent<component::ScriptComponent>()) return;

  auto& sc = entity.GetComponent<component::ScriptComponent>();
  const UUID scene_id = entity.GetSceneUUID();
  const UUID entity_id = entity.GetUUID();

  if (sc.is_runtime_initialized && sc.managed_instance != nullptr) {
    ShutdownRuntimeScriptEntity(entity);
    sc.managed_instance = nullptr;
  }

  if (erase && scene_state_.script_entities.contains(scene_id)) {
    scene_state_.field_map.erase(entity_id);
    sc.field_ids.clear();

    auto& script_entities = scene_state_.script_entities.at(scene_id);
    std::erase(script_entities, entity_id);
  }
}

MonoDomain* CSharpScriptEngine::GetCoreDomain() const {
  return mono_state_.domain_;
}

void CSharpScriptEngine::InitMono() {
  MonoGCManager::Init();

  mono_set_dirs("..\\vendor\\mono\\x86\\MonoAssembly\\bin\\",
                "..\\vendor\\mono\\x86\\MonoAssembly\\etc\\");

  mono_state_.root_domain_ = mono_jit_init("BaseEngine_ScriptCore");
  std::string domain_name = "BE_Runtime";
  mono_state_.domain_ =
      mono_domain_create_appdomain(domain_name.data(), nullptr);
  mono_domain_set(mono_state_.domain_, true);
  mono_domain_set_config(mono_state_.domain_, ".", "");
  LoadMonoAssembly();

  storage_ = std::make_unique<MonoScriptCacheStorage>();
  storage_->Init();
  storage_->GenerateCacheForAssembly(mono_state_.core_assembly_info_);

  glue_ = std::make_unique<MonoGlue>();
  glue_->RegisterGlue();
  auto obj = CSharpScriptEngine::GetInstance()->CreateManagedObject(
      CSharpScriptEngine::GetInstance()->GetManagedClassByName(
          "System.Diagnostics.StackTrace"));
  auto p = mono_object_get_class(obj);
  int n = 3;
}

CSharpScriptEngine::~CSharpScriptEngine() = default;

MonoImage* CSharpScriptEngine::GetCoreImage() {
  return mono_state_.core_assembly_info_->AssemblyImage;
}

void CSharpScriptEngine::SetScene(const Ref<Scene>& scene) {
  scene_state_.scene_context = scene;
}

Ref<Scene> CSharpScriptEngine::GetScene() { return scene_state_.scene_context; }

bool CSharpScriptEngine::IsScriptValid(AssetHandle script_asset_handle) const {
  if (!AssetManager::IsAssetHandleValid(script_asset_handle)) return false;

  if (this->mono_state_.core_assembly_info_ == nullptr) return false;

  Ref<Script> scriptAsset = AssetManager::GetAsset<Script>(script_asset_handle);
  return storage_->GetManagedClassById(scriptAsset->GetClassID()) != nullptr;
}

uint32_t CSharpScriptEngine::GetScriptClassIdFromComponent(
    const component::ScriptComponent& script) const {
  if (!AssetManager::IsAssetHandleValid(script.script_class_handle)) return 0;

  if (this->mono_state_.core_assembly_info_ == nullptr) return 0;

  Ref<Script> script_asset =
      AssetManager::GetAsset<Script>(script.script_class_handle);
  return script_asset->GetClassID();
}

MonoClassTypeInfo* CSharpScriptEngine::GetManagedClassByName(
    const std::string& class_name) const {
  return storage_->GetManagedClassByName(class_name);
}

MonoClassTypeInfo* CSharpScriptEngine::GetManagedClassById(
    const uint32_t class_id) const {
  return storage_->GetManagedClassById(class_id);
}

MonoClassTypeInfo* CSharpScriptEngine::GetMonoObjectClass(
    MonoObject* mono_object) const {
  return storage_->GetMonoObjectClass(mono_object);
}

MonoFieldInfo* CSharpScriptEngine::GetFieldById(uint32_t field_id) const {
  return storage_->GetFieldById(field_id);
}

Ref<IFieldStorage> CSharpScriptEngine::GetFieldStorage(
    const ObjectEntity entity, const uint32_t field_id) {
  const auto entity_id = entity.GetUUID();
  if (!scene_state_.field_map.contains(entity_id)) return nullptr;
  auto& field_map = scene_state_.field_map.at(entity_id);
  if (!field_map.contains(field_id)) return nullptr;

  return field_map[field_id];
}

void CSharpScriptEngine::InitRuntimeObject(MonoObject* mono_object) {
  mono_runtime_object_init(mono_object);
}

MonoObject* CSharpScriptEngine::CreateManagedObject(
    const MonoClassTypeInfo* managed_class) {
  MonoObject* mono_object =
      mono_object_new(mono_state_.domain_, managed_class->mono_class);
  return mono_object;
}

void CSharpScriptEngine::CallMethod(MonoObject* monoObject,
                                    MonoMethodInfo* managedMethod,
                                    const void** parameters) {
  MonoObject* exception = nullptr;
  mono_runtime_invoke(managedMethod->method, monoObject,
                      const_cast<void**>(parameters), &exception);
  if (exception != nullptr) {
    // TODO [ログ] monoの例外処理実装

    MonoClass* exception_class = mono_object_get_class(exception);
    MonoType* exception_type = mono_class_get_type(exception_class);
    std::string name = mono_type_get_name(exception_type);
    int n = 3;
  }
}
}  // namespace base_engine

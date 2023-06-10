// @CSharpScriptEngine.h
// @brief
// @author ICE
// @date 2023/04/06
//
// @details

#pragma once
#include <stack>
#include <unordered_map>

#include "IFieldStorage.h"
#include "MonoClassTypeInfo.h"
#include "MonoGCManager.h"
#include "MonoGlue.h"
#include "MonoScriptCash.h"
#include "Scene.h"
#include "ObjectEntity.h"
#include "ScriptComponent.h"
#include "ScriptTypes.h"

namespace base_engine {

class CSharpScriptEngine {
  static CSharpScriptEngine* instance_;

  CSharpScriptEngine(){};

 public:
  static CSharpScriptEngine* GetInstance() {
    if (!instance_) instance_ = new CSharpScriptEngine;
    return instance_;
  }

  void InitMono();

  void LoadMonoAssembly();

  void InitializeRuntime();
  void ShutdownRuntime();
  void ShutdownRuntimeScriptEntity(ObjectEntity entity);

  void InitializeScriptEntity(ObjectEntity entity);
  /**
   * \brief ランタイム時のScriptComponentをもったEntityの初期化を行う
   * \param entity
   */
  void RuntimeInitializeScriptEntity(ObjectEntity entity);

  void DuplicateScriptInstance(ObjectEntity entity, ObjectEntity target_entity);
  void InitializeRuntimeDuplicatedEntities();
	void ShutdownScriptEntity(ObjectEntity entity, bool erase = true);
  MonoDomain* GetCoreDomain() const;
  ;
  MonoImage* GetCoreImage();
  Ref<AssemblyInfo> GetCoreAssembly() {
    return mono_state_.core_assembly_info_;
  }

  void SetScene(const Ref<Scene>& scene);
  Ref<Scene> GetScene();

  bool IsScriptValid(AssetHandle script_asset_handle) const;
  uint32_t GetScriptClassIdFromComponent(
      const component::ScriptComponent& script) const;

  MonoClassTypeInfo* GetManagedClassByName(const std::string& class_name) const;
  MonoClassTypeInfo* GetManagedClassById(uint32_t class_id) const;
  MonoClassTypeInfo* GetMonoObjectClass(MonoObject* mono_object) const;
  MonoFieldInfo* GetFieldById(uint32_t field_id) const;
  Ref<IFieldStorage> GetFieldStorage(ObjectEntity entity, uint32_t field_id);

  MonoObject* CreateManagedObject(const MonoClassTypeInfo* managed_class);
  template <typename... TConstructorArgs>
  MonoObject* CreateManagedObject(uint32_t classID,
                                  TConstructorArgs&&... args) {
    return CreateManagedObjectImpl(this->storage_->GetManagedClassById(classID),
                                   std::forward<TConstructorArgs>(args)...);
  }
  template <typename... TConstructorArgs>
  MonoObject* CreateManagedObject(const std::string& name,
                                  TConstructorArgs&&... args) {
    return CreateManagedObjectImpl(this->storage_->GetManagedClassByName(name),
                                   std::forward<TConstructorArgs>(args)...);
  }

  static void CallMethod(MonoObject* monoObject, MonoMethodInfo* managedMethod,
                         const void** parameters);

  template <typename... TArgs>
  static void CallMethod(MonoObject* managedObject,
                         const std::string& methodName, TArgs&&... args) {
    if (managedObject == nullptr) {
      return;
    }

    constexpr size_t argsCount = sizeof...(args);

    MonoClassTypeInfo* clazz =
        GetInstance()->storage_->GetMonoObjectClass(managedObject);
    if (clazz == nullptr) {
      return;
    }

    MonoMethodInfo* method = GetInstance()->storage_->GetSpecificManagedMethod(
        clazz, methodName, argsCount);
    if (method == nullptr) {
      return;
    }

    if constexpr (argsCount > 0) {
      const void* data[] = {&args...};
      CallMethod(managedObject, method, data);
    } else {
      CallMethod(managedObject, method, nullptr);
    }
  }

  template <typename... TArgs>
  static void CallMethod(GCHandle instance, const std::string& methodName,
                         TArgs&&... args) {
    if (instance == nullptr) {
      return;
    }

    CallMethod(MonoGCManager::GetReferencedObject(instance), methodName,
               std::forward<TArgs>(args)...);
  }

  ~CSharpScriptEngine();

 private:
  struct MonoState {
    MonoDomain* root_domain_ = nullptr;
    MonoDomain* domain_ = nullptr;

    Ref<AssemblyInfo> core_assembly_info_;

    std::unordered_map<AssemblyMetadata, MonoAssembly*> ReferencedAssemblies;

  } mono_state_;
  using ScriptEntityMap = std::unordered_map<UUID, std::vector<UUID>>;
  using ScriptInstanceMap = std::unordered_map<UUID, GCHandle>;
  using FieldStorageMap = std::unordered_map<uint32_t, Ref<IFieldStorage>>;
  using InstanceFieldStorageMap = std::unordered_map<UUID, FieldStorageMap>;
  struct SceneState {
    Ref<Scene> scene_context;

    ScriptEntityMap script_entities;
    ScriptInstanceMap script_instances;
    InstanceFieldStorageMap field_map;
    std::stack<ObjectEntity> runtime_duplicated_script_entities;
  } scene_state_;
  std::unique_ptr<MonoScriptCacheStorage> storage_ = nullptr;
  std::unique_ptr<MonoGlue> glue_ = nullptr;

  void InitRuntimeObject(MonoObject* mono_object);

  template <typename... TConstructorArgs>
  MonoObject* CreateManagedObjectImpl(MonoClassTypeInfo* managedClass,
                                      TConstructorArgs&&... args) {
    if (managedClass == nullptr) return nullptr;

    MonoObject* obj = CreateManagedObject(managedClass);

    constexpr size_t argsCount = sizeof...(args);
    MonoMethodInfo* ctor =
        storage_->GetSpecificManagedMethod(managedClass, ".ctor", argsCount);

    InitRuntimeObject(obj);

    if constexpr (argsCount > 0) {
      if (ctor == nullptr) {
        // TODO [ログ][エラー処理]
        // 引数の数が一致するコンストラクタが存在しないことを伝える
        return obj;
      }

      const void* data[] = {&args...};
      CallMethod(obj, ctor, data);
    }

    return obj;
  }
};
}  // namespace base_engine
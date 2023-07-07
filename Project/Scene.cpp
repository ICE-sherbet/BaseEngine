#include "Scene.h"

#include <spdlog/spdlog.h>

#include "ApplyStaticGravitySystem.h"
#include "AssetManager.h"
#include "Audio.h"
#include "Becs/Registry.h"
#include "Becs/View.h"
#include "BodyTypeTag.h"
#include "BroadPhaseSystem.h"
#include "CSharpScriptEngine.h"
#include "ContactSolverSystem.h"
#include "ContactTesterCircleCircle.h"
#include "DataComponents.h"
#include "DynamicContactSolver.h"
#include "IBaseEngineAudioEngine.h"
#include "IBaseEngineRender.h"
#include "IntegratePosesSystem.h"
#include "MofTexture.h"
#include "ObjectEntity.h"
#include "PhysicsContactListenerSystem.h"
#include "Prefab.h"
#include "Profiler.h"
#include "ShapeComponents.h"
#include "SpriteRendererComponent.h"
#include "Texture.h"
#include "UpdateBV.h"
#include "UpdateCirclesBV.h"
using namespace base_engine;
using namespace component;

ObjectEntity Scene::CreateEntity(const std::string& name) {
  return CreateChildEntity({}, name);
}

ObjectEntity Scene::CreateChildEntity(const ObjectEntity parent,
                                      const std::string& name) {
  const auto handle = registry_.create();
  ObjectEntity entity{handle, this};
  auto& [uuid] = entity.AddComponent<IdComponent>();

  auto& transform = entity.AddComponent<TransformComponent>();
  transform.SetScene(this);
  if (!name.empty()) {
    entity.AddComponent<TagComponent>(name);
  } else {
    entity.AddComponent<TagComponent>("GameObject");
  }
  entity.AddComponent<HierarchyComponent>();

  if (parent) entity.SetParent(parent);

  entity_id_map_[uuid] = entity;
  object_entity_map_[uuid] = entity;
  return entity;
}

ObjectEntity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
  const auto entity = ObjectEntity{registry_.create(), this};
  auto& [entity_uuid] = entity.AddComponent<IdComponent>();
  entity_uuid = uuid;
  auto& transform = entity.AddComponent<TransformComponent>();
  transform.SetScene(this);
  if (!name.empty()) entity.AddComponent<TagComponent>(name);

  entity.AddComponent<HierarchyComponent>();

  BE_CORE_ASSERT(!this->entity_id_map_.contains(uuid));
  entity_id_map_[uuid] = entity;
  object_entity_map_[uuid] = entity;
  return entity;
}

ObjectEntity Scene::CreatePrefabEntity(ObjectEntity entity, ObjectEntity parent,
                                       const Vector3* translation,
                                       const Vector3* rotation,
                                       const Vector3* scale) {
  ObjectEntity new_entity = CreateEntity();

  entity.GetScene()->GetRegistry().copy_to<IdComponent>(entity, registry_,
                                                        new_entity);

  auto&& transform = new_entity.GetComponent<TransformComponent>();
  transform.SetChildren(std::vector<UUID>{});
  transform.SetScene(this);
  new_entity.Children().clear();
  if (parent) {
    transform.SetParent(new_entity.GetUUID());
    new_entity.SetParent(parent);
  }

  if (translation) transform.SetGlobalTranslation(*translation);
  if (rotation) transform.SetLocalRotationEuler(*rotation);
  if (scale) transform.SetLocalScale(*scale);

  for (const auto child_id : entity.Children())
    CreatePrefabEntity(entity.scene_->GetEntityWithUUID(child_id), new_entity);

  if (new_entity.HasComponent<ScriptComponent>())
    CSharpScriptEngine::GetInstance()->DuplicateScriptInstance(entity,
                                                               new_entity);

  return new_entity;
}

ObjectEntity Scene::Instantiate(Ref<Prefab>& prefab, const Vector3* translation,
                                const Vector3* rotation, const Vector3* scale) {
  ObjectEntity result;

  for (const auto entities =
           prefab->scene_->GetAllEntitiesWith<HierarchyComponent>();
       const auto e : entities) {
    ObjectEntity entity = {e, prefab->scene_.Raw()};
    if (entity.GetParent()) continue;
    result = CreatePrefabEntity(entity, {}, translation, rotation, scale);
    break;
  }

  return result;
}

ObjectEntity Scene::GetEntityWithUUID(const UUID id) const {
  return object_entity_map_.at(id);
}

ObjectEntity Scene::TryGetEntityWithUUID(const UUID id) const {
  if (const auto iter = entity_id_map_.find(id); iter != entity_id_map_.end())
    return object_entity_map_.at(id);
  return ObjectEntity{};
}

ObjectEntity Scene::FindEntityWithTag(const std::string& tag) const {
  for (const auto entities = registry_.view<TagComponent>();
       const auto entity : entities) {
    if (entities.get<TagComponent>(entity).tag == tag) {
      return ObjectEntity{entity, const_cast<Scene*>(this)};
    }
  }
  return ObjectEntity{};
}

void Scene::DestroyEntity(UUID id) {
  if (!this->object_entity_map_.contains(id)) return;
  DestroyEntity(this->object_entity_map_.at(id));
}

void Scene::DestroyEntity(ObjectEntity entity) {
  {
    for (const auto child : entity.Children()) {
      auto child_entity = TryGetEntityWithUUID(child);
      ConvertToWorldSpace(child_entity);
      child_entity.SetParentUUID(0);
    }
  }
  object_entity_map_.erase(entity.GetUUID());
  entity_id_map_.erase(entity.GetUUID());

  registry_.destroy(entity.GetHandle());
}

void Scene::UnParentEntity(const ObjectEntity entity) const {
  UnParentEntity(entity, true);
}

void Scene::UnParentEntity(ObjectEntity entity,
                           const bool world_position_stays) const {
  auto parent = TryGetEntityWithUUID(entity.GetParentUUID());
  if (!parent) return;

  std::vector<UUID>& parent_children = parent.Children();
  auto result = std::ranges::remove(parent_children, entity.GetUUID());
  parent_children.erase(result.begin(), parent_children.end());
  parent.GetComponent<component::TransformComponent>().SetChildren(
      parent_children);

  if (world_position_stays) ConvertToWorldSpace(entity);

  entity.SetParentUUID(0);
}

void Scene::ConvertToWorldSpace(ObjectEntity entity) const {
  if (const ObjectEntity parent = TryGetEntityWithUUID(entity.GetParentUUID());
      !parent)
    return;

  const auto transform = GetWorldSpaceTransformMatrix(entity);
  auto& entity_transform = entity.GetComponent<TransformComponent>();
  entity_transform.SetLocalTransform(transform);
}
namespace detail {
bool IsGlobalDirty(ObjectEntity entity) {
  TransformComponent& transform = entity.GetComponent<TransformComponent>();
  const auto dirty = entity.GetComponent<TransformComponent>().IsDirty();
  if (const auto parent = entity.GetParent()) {
    const auto parent_dirty = IsGlobalDirty(parent);
    if (parent_dirty) {
      transform.SetDirty(true);
    }
    return parent_dirty || dirty;
  } else {
    return dirty;
  }
}
}  // namespace detail
Matrix44 Scene::InternalGetWorldSpaceTransformMatrix(
    ObjectEntity entity) const {
  auto& transform = entity.GetComponent<TransformComponent>();
  if (transform.IsDirty()) {
    if (const auto parent = entity.GetParent()) {
      transform.SetGlobalTransform(
          transform.GetLocalTransform() *
          InternalGetWorldSpaceTransformMatrix(parent));
    } else {
      transform.SetGlobalTransform(transform.GetLocalTransform());
    }
    for (auto&& child : entity.Children()) {
      if (auto child_entity = TryGetEntityWithUUID(child); child_entity) {
        child_entity.GetComponent<TransformComponent>().SetDirty(true);
      }
    }
    transform.SetDirty(false);
  }
  return transform.GetGlobalTransform();
}

Matrix44 Scene::GetWorldSpaceTransformMatrix(const ObjectEntity entity) const {
  BE_PROFILE_FUNC();
  return InternalGetWorldSpaceTransformMatrix(entity);
}

TransformComponent Scene::GetWorldSpaceTransform(
    const ObjectEntity entity) const {
  const auto transform = GetWorldSpaceTransformMatrix(entity);
  TransformComponent transform_component;
  transform_component.SetLocalTransform(transform);
  return transform_component;
}

void Scene::OnInit() {
  const auto physics_engine_data = Ref<physics::PhysicsEngineData>::Create();
  systems_.emplace_back(std::make_unique<physics::ApplyStaticGravitySystem>(
      this, physics_engine_data));
  systems_.emplace_back(std::make_unique<physics::IntegratePosesSystem>(
      this, physics_engine_data));
  systems_.emplace_back(
      std::make_unique<physics::UpdateCirclesBV>(this, physics_engine_data));

  // BroadPhase
  systems_.emplace_back(
      std::make_unique<physics::BroadPhaseSystem>(this, physics_engine_data));

  // NarrowPhase
  systems_.emplace_back(std::make_unique<physics::ContactTesterCircleCircle>(
      this, physics_engine_data));

  // SolverPhase
  systems_.emplace_back(
      std::make_unique<
          physics::DynamicContactSolver<physics::Dynamic, physics::Dynamic>>(
          this, physics_engine_data));

  // ListenerPhase
  systems_.emplace_back(std::make_unique<physics::PhysicsContactListenerSystem>(
      this, physics_engine_data));

  for (const auto& system : systems_) {
    system->OnInit();
  }
}

void Scene::ScriptOnUpdate(float time) {
  for (const auto view = registry_.view<IdComponent, ScriptComponent>();
       const auto entity : view) {
    if (auto [id, scriptComponent] =
            view.get<IdComponent, ScriptComponent>(entity);
        scriptComponent.is_runtime_initialized) {
      CSharpScriptEngine::CallMethod(scriptComponent.managed_instance,
                                     "OnUpdate", time);
    } else {
      // TODO エラー処理
    }
  }
}

void Scene::AudioOnPlaying() {
  for (const auto view = registry_.view<IdComponent, AudioComponent>();
       const auto entity : view) {
    auto [id, audio_component] = view.get<IdComponent, AudioComponent>(entity);
    if (!audio_component.audio_source) continue;
    if (audio_component.is_playing) continue;
    if (!AssetManager::IsAssetHandleValid(audio_component.audio_source))
      continue;

    if (const auto sound =
            AssetManager::GetAsset<Audio>(audio_component.audio_source);
        BASE_ENGINE(AudioEngine)->Play(sound->GetBuffer())) {
      audio_component.is_playing = true;
    }
  }
}

void Scene::PhysicsUpdate() {
  BE_PROFILE_FUNC("PhysicsUpdate");
  for (const auto& system : systems_) {
    system->OnUpdate();
  }
}

void Scene::OnUpdate(const float time) {
  PhysicsUpdate();
  ScriptOnUpdate(time);
  CSharpScriptEngine::GetInstance()->InitializeRuntimeDuplicatedEntities();
}

void Scene::OnUpdateRuntime(float time) {}

void Scene::OnUpdateEditor(float time) {}

void Scene::OnRenderRuntime(float time) {}

void Scene::OnRenderEditor(float time) {}

void Scene::OnRuntimeStart() {
  is_playing_ = true;
  CSharpScriptEngine::GetInstance()->InitializeRuntime();
}

void Scene::OnRuntimeStop() {
  is_playing_ = false;
  CSharpScriptEngine::GetInstance()->ShutdownRuntime();
}

void Scene::OnRender(float time) {
  for (const auto view =
           registry_.view<TransformComponent, SpriteRendererComponent>();
       const auto entity : view) {
    auto [transform, spriteRendererComponent] =
        view.get<TransformComponent, SpriteRendererComponent>(entity);
    if (!spriteRendererComponent.texture) {
      continue;
    }
    if (AssetManager::IsAssetHandleValid(spriteRendererComponent.texture)) {
      Ref<MofTexture> texture =
          AssetManager::GetAsset<MofTexture>(spriteRendererComponent.texture);
      BASE_ENGINE(Render)->AddTexture(
          texture->texture_, transform.GetGlobalTransform(),
          {0, 0, static_cast<MofFloat>(texture->texture_->GetWidth()),
           static_cast<MofFloat>(texture->texture_->GetHeight())},
          Mof::CVector4Utilities::ToU32Color(spriteRendererComponent.color),
          {spriteRendererComponent.Pivot().x, spriteRendererComponent.Pivot().y,
           0});

    } else {
      BE_CORE_INFO("UUID:{0} テクスチャデータの参照がありません。",
                   spriteRendererComponent.texture);
    }
  }
}

Scene::Scene() {
  object_entity_map_ = ObjectEntityMap{internal::kDefaultCapacity};
}

template <typename T>
static void CopyComponent(
    becs::registry& dst_registry, becs::registry& src_registry,
    const std::unordered_map<base_engine::UUID, becs::Entity>& entity_map) {
  for (auto src_entities = src_registry.view<T>();
       auto src_entity : src_entities) {
    becs::Entity dest_entity = entity_map.at(
        src_registry.get<component::IdComponent>(src_entity).uuid);

    T src_component = src_registry.get<T>(src_entity);
    auto& dest_component =
        dst_registry.emplace_or_replace<T>(dest_entity, src_component);
  }
}

void Scene::CopyTo(Scene* to) {
  to->scene_name_ = scene_name_;

  std::unordered_map<UUID, becs::Entity> entity_map;
  const auto id_components = registry_.view<IdComponent>();
  for (const auto entity : id_components) {
    auto uuid = registry_.get<IdComponent>(entity).uuid;
    auto name = registry_.get<TagComponent>(entity).tag;
    const auto new_entity = to->CreateEntityWithUUID(uuid, name);
    entity_map[uuid] = new_entity.entity_handle_;
  }
  CopyComponent<TransformComponent>(to->registry_, registry_, entity_map);
  for (const auto view = to->registry_.view<TransformComponent>();
       const auto e : view) {
    auto& transform = to->registry_.get<TransformComponent>(e);
    transform.SetScene(to);
  }

  CopyComponent<TagComponent>(to->registry_, registry_, entity_map);
  CopyComponent<HierarchyComponent>(to->registry_, registry_, entity_map);
  CopyComponent<ScriptComponent>(to->registry_, registry_, entity_map);
  CopyComponent<SpriteRendererComponent>(to->registry_, registry_, entity_map);
  CopyComponent<physics::RigidBodyComponent>(to->registry_, registry_,
                                             entity_map);
  CopyComponent<physics::VelocityComponent>(to->registry_, registry_,
                                            entity_map);
  CopyComponent<physics::BodyMask>(to->registry_, registry_, entity_map);
  CopyComponent<physics::CircleShape>(to->registry_, registry_, entity_map);
  CopyComponent<physics::BoundingBox>(to->registry_, registry_, entity_map);
}

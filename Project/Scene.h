// @Scene.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include "Asset.h"
#include "Becs/Entity.h"
#include "Becs/Registry.h"
#include "IBecsSystem.h"
#include "Matrix44.h"
#include "ObjectEntity.h"
#include "Ref.h"
#include "UUID.h"

namespace base_engine {
class Prefab;
using EntityMap = std::unordered_map<UUID, becs::Entity>;
using ObjectEntityMap = std::unordered_map<UUID, ObjectEntity>;
namespace internal {
constexpr size_t kDefaultCapacity = 1024;
}
class Scene final : public Asset {
 public:
  /**
   * \brief 親を持たない Entity を生成する
   * \param name Entity 名
   * \return 生成した Entity が返される
   */
  ObjectEntity CreateEntity(const std::string& name = "");
  ObjectEntity CreateChildEntity(ObjectEntity parent,
                                 const std::string& name = "");
  ObjectEntity CreateEntityWithUUID(UUID uuid, const std::string& name = "");

  template <typename... Components>
  [[nodiscard]] auto GetAllEntitiesWith() {
    return registry_.view<Components...>();
  }
  template <typename... Components>
  [[nodiscard]] auto GetAllEntitiesWith() const {
    return registry_.view<Components...>();
  }
  ObjectEntity CreatePrefabEntity(ObjectEntity entity, ObjectEntity parent,
                                  const Vector3* translation = nullptr,
                                  const Vector3* rotation = nullptr,
                                  const Vector3* scale = nullptr);

  ObjectEntity Instantiate(Ref<Prefab>& prefab,
                           const Vector3* translation = nullptr,
                           const Vector3* rotation = nullptr,
                           const Vector3* scale = nullptr);

  becs::registry& GetRegistry() { return registry_; }
  ObjectEntity GetEntityWithUUID(UUID id) const;
  ObjectEntity TryGetEntityWithUUID(UUID id) const;

  ObjectEntity FindEntityWithTag(const std::string& tag) const;
  void DestroyEntity(UUID id);
  void DestroyEntity(ObjectEntity entity);

  void UnParentEntity(ObjectEntity entity) const;
  void UnParentEntity(ObjectEntity entity, bool world_position_stays) const;
  void SetParentEntity(ObjectEntity entity, ObjectEntity parent) const;
  void SetParentEntity(ObjectEntity entity, ObjectEntity parent,
                       bool world_position_stays) const;

  void ConvertToWorldSpace(ObjectEntity entity) const;
  void ConvertToLocalSpace(ObjectEntity entity) const;

  Matrix44 GetWorldSpaceTransformMatrix(const ObjectEntity entity) const;

  component::TransformComponent GetWorldSpaceTransform(
      const ObjectEntity entity) const;

  void OnInit();
  void OnUpdate(float time);
  void OnUpdateRuntime(float time);
  void OnUpdateEditor(float time);

  void OnRender(float time);
  void OnRenderRuntime(float time);
  void OnRenderEditor(float time);

  UUID GetUUID() const { return scene_id_; }
  std::string_view GetName() const { return scene_name_; }
  void SetName(const std::string& name) { scene_name_ = name; }
  bool IsPlaying() const { return is_playing_; }

  Scene();
  Scene(const std::string& name) : scene_name_(name) {}
  void CopyTo(Scene* to);

  void OnRuntimeStart();
  void OnRuntimeStop();

 private:
  UUID scene_id_;
  std::string scene_name_;

  becs::Entity scene_entity_ = becs::kNull;
  becs::registry registry_;
  EntityMap entity_id_map_{internal::kDefaultCapacity};
  ObjectEntityMap object_entity_map_;
  friend ObjectEntity;

  std::vector<std::unique_ptr<ISystem>> systems_;
  bool is_playing_ = false;
  /**
   * \brief 各Entityが持つScriptComponentのOnUpdateを呼び出す
   * \param time 前回のUpdateからの経過時間
   */
  void ScriptOnUpdate(float time);

  void AudioOnPlaying();
  void PhysicsUpdate();

  Matrix44 InternalGetWorldSpaceTransformMatrix(
      const ObjectEntity entity) const;
};


template <typename T, typename... Args>
  requires std::is_constructible_v<T, Args...>
T& ObjectEntity::AddComponent(Args&&... args) const {
  BE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
  if (HasComponent<T>()) {
    __debugbreak();
  }
  return scene_->registry_.emplace<T>(entity_handle_,
                                      std::forward<Args>(args)...);
}

template <typename T>
T& ObjectEntity::GetComponent() {
  BE_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
  if (!HasComponent<T>()) {
    __debugbreak();
  }
  return scene_->registry_.get<T>(entity_handle_);
}

template <typename T>
const T& ObjectEntity::GetComponent() const {
  BE_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
  if (!HasComponent<T>()) {
    __debugbreak();
  }
  return scene_->registry_.get<T>(entity_handle_);
}

template <typename... T>
bool ObjectEntity::HasComponent() const {
  return scene_->registry_.all_of<T...>(entity_handle_);
}
}  // namespace base_engine
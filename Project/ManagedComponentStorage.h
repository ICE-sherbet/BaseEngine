// @ManagedComponentStorage.h
// @brief
// @author ICE
// @date 2023/04/17
//
// @details

#pragma once
#include "ObjectEntity.h"
#include "ScriptTypes.h"

namespace base_engine {
class ManagedComponentStorage {
 public:
  static void Init() {
    if (instance_) return;
    instance_ = new ManagedComponentStorage();
  }
  static ManagedComponentStorage* GetInstance() { return instance_; }
  template <typename TComponent>
  void RegisterComponent(MonoType* managed_type) {
    create_component_functions_[managed_type] = [](ObjectEntity& entity) {
      entity.AddComponent<TComponent>();
    };
    has_component_functions_[managed_type] = [](ObjectEntity& entity) {
      return entity.HasComponent<TComponent>();
    };
    remove_component_functions_[managed_type] = [](ObjectEntity& entity) {
      // TODO Removeを実装し追加
      // entity.RemoveComponent<TComponent>();
    };
  }
  void Clear() {
    create_component_functions_.clear();
    has_component_functions_.clear();
    remove_component_functions_.clear();
  }
  bool Contains(MonoType* managed_type) const {
    return has_component_functions_.contains(managed_type);
  }

  bool HasComponent(MonoType* managed_type, ObjectEntity& entity) const {
    return has_component_functions_.at(managed_type)(entity);
  }
  void CreateComponent(MonoType* managed_type, ObjectEntity& entity) const {
    create_component_functions_.at(managed_type)(entity);
  }

 private:
  static ManagedComponentStorage* instance_;

  std::unordered_map<MonoType*, std::function<void(ObjectEntity&)>>
      create_component_functions_;
  std::unordered_map<MonoType*, std::function<bool(ObjectEntity&)>>
      has_component_functions_;
  std::unordered_map<MonoType*, std::function<void(ObjectEntity&)>>
      remove_component_functions_;
};
}  // namespace base_engine

﻿#pragma once

#include <algorithm>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "Game.h"
#include "IBaseEngineCollider.h"
#include "Vector2.h"
namespace base_engine {
using ComponentPtr = std::shared_ptr<class Component>;
using ComponentWeakPtr = std::weak_ptr<class Component>;
template <class T>
using ComponentDerivedPtr = std::shared_ptr<T>;
template <class T>
using ComponentDerivedWeakPtr = std::weak_ptr<T>;

struct ActorId {
  uint64_t id;

  friend bool operator==(const ActorId& lhs, const ActorId& rhs) {
    return lhs.id == rhs.id;
  }

  friend bool operator!=(const ActorId& lhs, const ActorId& rhs) {
    return !(lhs == rhs);
  }
};

class Actor {
  friend class Game;

 public:
  enum State { kStart, kActive, kPause, kDead };

  Actor(class Game* game);
  Actor(class Game* game, std::weak_ptr<GameScene> scene);
  virtual ~Actor();

  void StartActor();
  /**
   * \brief Actorを継承した各クラスが登録された後に呼び出される
   */
  virtual void Start() {}
  void ProcessInput();
  /**
   * \brief Update前に毎フレーム呼び出される
   */
  virtual void Input() {}

  void UpdateActor();
  /**
   * \brief 毎フレーム呼び出される
   */
  virtual void Update() {}

  template <class T>
  void SendCallbackMessage(T* callback) {
    for (auto element : components_) {
      callback->SendComponentsMessage(element);
    }
  }

  template <class T, class D>
  void SendCallbackMessage(T* callback, D&& data) {
    for (auto element : components_) {
      callback->SendComponentsMessage(element.get(), data);
    }
  }
  [[nodiscard]] State GetState() const { return state_; }
  void SetState(const State state) { state_ = state; }

  [[nodiscard]] const Vector2& GetPosition() const { return position_; }
  void SetPosition(const Vector2& pos) { position_ = pos; }
  void Translation(const Vector2& pos) { position_ += pos; }
  float GetRotation() const;
  void SetRotation(const float rotation) { rotation_ = rotation; }
  [[nodiscard]] float GetScale() const { return scale_; }
  void SetScale(const float scale) { scale_ = scale; }

  [[nodiscard]] Game* GetGame() const { return game; }
  /**
   * \brief Componentの追加はコンストラクタから呼び出す
   * \param component
   */
  void AddComponent(class Component* component);
  /**
   * \brief コンポーネントを登録する
   */
  void AddComponent();
  /**
   * \brief コンポーネント型を探索して取得する
   * \tparam T Componentクラスを継承したクラス
   * \return 成功:最初に見つかったコンポーネントのポインタ, 失敗:nullptr
   * 同一型のコンポーネントが複数ある場合同じものが返ってくるとは限らない
   */
  template <class T>
  [[nodiscard]] ComponentDerivedWeakPtr<T> GetComponent() const;

  /**
   * \brief 指定のコンポーネントを全て探索して取得する
   * \tparam T Componentクラスを継承したクラス
   * \return 成功:探索したコンポーネントのポインタ,失敗:nullptr
   */
  template <class T>
  std::vector<ComponentDerivedWeakPtr<T>> GetComponents() const;
  /**
   * \brief Componentのデストラクタから呼び出す
   * \param component
   */
  void RemoveComponent(class Component* component);
  std::string_view GetName() { return name_; }
  Actor& SetName(const std::string_view name);

  void SetTag(std::string_view tag);
  [[nodiscard]] std::string_view GetTag() const;

  [[nodiscard]] ActorId GetId() const { return id_; }

  [[nodiscard]] const std::list<std::weak_ptr<Actor>>& GetChildren() const {
    return children_;
  }

  [[nodiscard]] std::weak_ptr<Actor> GetChildByName(
      const std::string_view name) const;
  bool RemoveChild(const ActorId id);

  void AddChild(const std::weak_ptr<Actor>& actor);

  void AddChild(const ActorId& id);
  [[nodiscard]] std::weak_ptr<Actor> GetChildByTag(
      const std::string_view tag) const;

  [[nodiscard]] std::weak_ptr<Actor> GetParent() const;
  [[nodiscard]] std::weak_ptr<GameScene> GetScene() const { return scene_; }
  void SetScene(const std::weak_ptr<GameScene> scene) { scene_ = scene; }

  [[nodiscard]] bool Enable() const { return b_enable_; }

  void SetEnable(bool b_enable);
  virtual void OnEnable() {}
  virtual void OnDisable() {}

 protected:
  std::string name_ = "Actor";
  std::string tag_ = "Object";
  State state_;
  Vector2 position_;
  float rotation_;
  float scale_;
  bool b_enable_ = true;

 private:
  ActorId id_{};
  class Game* game;
  std::vector<ComponentPtr> components_;
  std::vector<ComponentPtr> pending_components_;

  std::list<std::weak_ptr<Actor>> children_;
  std::weak_ptr<Actor> parent_;
  std::weak_ptr<GameScene> scene_;
};

template <class T>
ComponentDerivedWeakPtr<T> Actor::GetComponent() const {
  for (auto& elem : components_) {
    if (ComponentDerivedPtr<T> buff = std::dynamic_pointer_cast<T>(elem))
      return buff;
  }
  for (auto& elem : pending_components_) {
    if (ComponentDerivedPtr<T> buff = std::dynamic_pointer_cast<T>(elem))
      return buff;
  }
  return ComponentDerivedWeakPtr<T>();
}

template <class T>
std::vector<ComponentDerivedWeakPtr<T>> Actor::GetComponents() const
{
  std::vector<ComponentDerivedWeakPtr<T>> result;
  for (auto& elem : components_) {
    if (ComponentDerivedPtr<T> buff = std::dynamic_pointer_cast<T>(elem))
      result.emplace_back(buff);
  }
  for (auto& elem : pending_components_) {
    if (ComponentDerivedPtr<T> buff = std::dynamic_pointer_cast<T>(elem))
      result.emplace_back(buff);
  }
  return result;
}

}  // namespace base_engine
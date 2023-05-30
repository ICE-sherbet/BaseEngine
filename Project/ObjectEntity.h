// @ObjectEntity.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <string_view>

#include "DataComponents.h"
#include "Scene.h"
namespace base_engine {

class ObjectEntity {
 public:
  ObjectEntity() = default;
  ObjectEntity(const becs::Entity handle, Scene* scene)
      : entity_handle_(handle), scene_(scene) {}

  ~ObjectEntity() = default;

  /**
   * \brief コンポーネントを構築し、追加を行う。
   * \warning
   * 引数には作成したいコンポーネントのコンストラクタと同じ引数を渡しましょう。
   * \tparam T コンポーネント型
   * \tparam Args 構築引数型
   * \param args コンポーネントを構築するための引数
   * \return 構築されたコンポーネント
   */
  template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
  T& AddComponent(Args&&... args) const {
    BE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
    if (HasComponent<T>()) {
      int n = 3;
    }
    return scene_->registry_.emplace<T>(entity_handle_,
                                        std::forward<Args>(args)...);
  }

  /**
   * \brief 自身のEntityが所持しているコンポーネントの取得を行う
   * \tparam T 取得したいコンポーネント型
   * \return 指定のコンポーネント
   */
  template <typename T>
  T& GetComponent() {
    BE_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
    if (!HasComponent<T>())
    {
      int n = 3;
    }
    return scene_->registry_.get<T>(entity_handle_);
  }

  template <typename T>
  [[nodiscard]] const T& GetComponent() const {
    BE_CORE_ASSERT(HasComponent<T>(), "Entity doesn't have component!");
    if (!HasComponent<T>()) {
      int n = 3;
    }
    return scene_->registry_.get<T>(entity_handle_);
  }
  template <typename... T>
  [[nodiscard]] bool HasComponent() const {
    return scene_->registry_.all_of<T...>(entity_handle_);
  }
  operator uint32_t() const { return static_cast<uint32_t>(entity_handle_); }
  operator becs::Entity() const { return entity_handle_; }
  operator bool() const { return (entity_handle_ != becs::null) && scene_; }
  bool operator==(const ObjectEntity& other) const {
    return entity_handle_ == other.entity_handle_ && scene_ == other.scene_;
  }
  bool operator!=(const ObjectEntity& other) const { return !(*this == other); }

  [[nodiscard]] ObjectEntity GetParent() const {
    return scene_->TryGetEntityWithUUID(GetParentUUID());
  }

  [[nodiscard]] UUID GetParentUUID() const {
    return GetComponent<component::HierarchyComponent>().parent_handle;
  }

  bool RemoveChild(ObjectEntity child) {
    const UUID child_id = child.GetUUID();
    std::vector<UUID>& children = Children();
    if (const auto it = std::ranges::find(children, child_id);
        it != children.end()) {
      children.erase(it);
      return true;
    }
    GetComponent<component::TransformComponent>().SetChildren(children);
    return false;
  }

  /**
   * \brief 親を設定する。
   * 既に親として登録されていた親Entityの子要素からは削除される
   * \param parent 親
   */
  void SetParent(ObjectEntity parent);
  /**
   * \brief 親を設定する
   * 既に親として登録されていた親Entityの子要素からは削除される
   * \param parent 親
   * \param world_position_stays もし true の場合、ワールド座標を維持し、親要素からの相対的な位置などが変更される。
   */
  void SetParent(ObjectEntity parent, bool world_position_stays);

  void SetParentUUID(const UUID& parent);

  std::vector<UUID>& Children() {
    return GetComponent<component::HierarchyComponent>().children;
  }

  [[nodiscard]] const std::vector<UUID>& Children() const {
    return GetComponent<component::HierarchyComponent>().children;
  }
  [[nodiscard]] UUID GetUUID() const {
    return GetComponent<component::IdComponent>().uuid;
  }
  [[nodiscard]] UUID GetSceneUUID() const { return scene_->GetUUID(); }
  [[nodiscard]] Ref<Scene> GetScene() const { return scene_; }

  [[nodiscard]] becs::Entity GetHandle() const { return entity_handle_; }

 private:
  /**
   * \brief シリアライズされたデータからシーンで生成される場合などに使用する
   * \param name オブジェクト名
   */
  //  ObjectEntity(const std::string& name){};

 private:
  becs::Entity entity_handle_{becs::kNull};
  Scene* scene_ = nullptr;

  friend class Scene;

  /**
   * \brief オプションとして、無名オブジェクトの命名法を変える可能性があるため
   * 定数 にはしない
   */
  inline static std::string NoName = "No Name";
};
}  // namespace base_engine
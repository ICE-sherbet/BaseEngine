// @ObjectEntity.h
// @brief
// @author ICE
// @date 2023/03/07
//
// @details

#pragma once
#include <string_view>

#include "DataComponents.h"
namespace base_engine {
class Scene;

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
  T& AddComponent(Args&&... args) const;

  /**
   * \brief 自身のEntityが所持しているコンポーネントの取得を行う
   * \tparam T 取得したいコンポーネント型
   * \return 指定のコンポーネント
   */
  template <typename T>
  T& GetComponent();

  template <typename T>
  [[nodiscard]] const T& GetComponent() const;

  template <typename... T>
  [[nodiscard]] bool HasComponent() const;
  operator uint32_t() const { return static_cast<uint32_t>(entity_handle_); }
  operator becs::Entity() const { return entity_handle_; }
  operator bool() const { return (entity_handle_ != becs::null) && scene_; }
  bool operator==(const ObjectEntity& other) const {
    return entity_handle_ == other.entity_handle_ && scene_ == other.scene_;
  }
  bool operator!=(const ObjectEntity& other) const { return !(*this == other); }

  [[nodiscard]] ObjectEntity GetParent() const;

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
  [[nodiscard]] UUID GetSceneUUID() const;
  [[nodiscard]] Scene* GetScene() const { return scene_; }

  [[nodiscard]] becs::Entity GetHandle() const { return entity_handle_; }

  /**
   * \brief Scriptを含むプロパティのリストを取得する
   * \param list OUT 全てのプロパティリストが返される
   */
  void GetPropertyList(std::list<PropertyInfo>* list);


  /**
   * \brief Scriptを含む指定クラスのプロパティリストを取得する
   * \param class_name 指定クラスの名前
   * \param list OUT プロパティのリストが返される
   */
  void GetClassPropertyList(const std::string& class_name,
                            std::list<PropertyInfo>* list);
  /**
   * \brief Scriptを含む指定クラスのプロパティリストを取得する
   * \param class_id 指定クラスのハッシュ値
   * \param list OUT プロパティのリストが返される
   */
  void GetClassPropertyList(uint32_t class_id, std::list<PropertyInfo>* list);

  /**
   * \brief Scriptを含む指定クラスのプロパティを取得する
   * \param class_name 指定クラスの名前
   * \param property_info プロパティ情報
   * \param return_value プロパティのゲッターの戻り値
   * \return プロパティを取得できたかどうか
   */
  bool TryGetProperty(const std::string& class_name,
                      const PropertyInfo& property_info,
                             Variant& return_value) const;
  /**
   * \brief Scriptを含む指定クラスのプロパティを取得する
   * \param class_name 指定クラスの名前
   * \param property_name プロパティ名前
   * \param return_value プロパティのゲッターの戻り値
   * \return プロパティを取得できたかどうか
   */
  bool TryGetProperty(const std::string& class_name,
                      const std::string& property_name,
                      Variant& return_value) const;

  /**
   * \brief Scriptを含む指定クラスのプロパティにセットする
   * \param class_name 指定クラスの名前
   * \param property_info プロパティ情報
   * \param value プロパティのセッターに渡す値
   * \return プロパティにセット出来たかどうか
   */
  bool TrySetProperty(const std::string& class_name,
                      const PropertyInfo& property_info,
                      const Variant& value);
  /**
   * \brief Scriptを含む指定クラスのプロパティにセットする
   * \param class_name 指定クラスの名前
   * \param property_name プロパティ名前
   * \param value プロパティのセッターに渡す値
   * \return プロパティにセット出来たかどうか
   */
  bool TrySetProperty(const std::string& class_name,
                      const std::string& property_name,
                      const Variant& value);

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
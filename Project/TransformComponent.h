// @TransformComponent.h
// @brief
// @author ICE
// @date 2023/03/19
//
// @details

#pragma once
#include "MathUtilities.h"
#include "Matrix44.h"
#include "Matrix44Utilities.h"
#include "QuaternionUtilities.h"
#include "UUID.h"
#include "Vector2.h"
#include "Vector3.h"

#include "ComponentDB.h"
#include "ComponentProperty.h"
#include "Asset.h"

namespace base_engine {
class Scene;
}

namespace base_engine::component {
// TODO [優先度 低] TransformComponentの実装を見直す。
// トポロジカルソートなどで親子関係の依存を解決できるか検討する

class TransformComponent {
 public:
  BE_COMPONENT(TransformComponent)

  TransformComponent();
  TransformComponent(const TransformComponent& other) = default;

  TransformComponent(const Vector3& translation);

  [[nodiscard]] Matrix44 GetLocalTransform() const;
  void SetLocalTransform(const Matrix44& transform);

  void SetLocalTranslation(const Vector3& pos);
  Vector3 GetLocalTranslation() const;

  void SetLocalScale(const Vector3& scale);
  Vector3 GetLocalScale() const;

  void SetLocalRotationEuler(const Vector3& euler);
  Vector3 GetLocalRotationEuler() const;

  void SetLocalRotation(const Quaternion& quat);
  Quaternion GetLocalRotation();

  [[nodiscard]] Matrix44 GetGlobalTransform();
  void SetGlobalTransform(const Matrix44& transform);

  void SetGlobalTranslation(const Vector3& pos);
  Vector3 GetGlobalTranslation() const;

  Vector3 GetGlobalRotationEuler() const;

  [[deprecated(
      "本来の設計思想とマッチしないため、削除される可能性が高いです")]] bool
  IsDirty() const {
    return global_dirty_;
  }
  [[deprecated(
      "本来の設計思想とマッチしないため、削除される可能性が高いです")]] void
  SetDirty(const bool dirty) {
    global_dirty_ = dirty;
  }
  [[deprecated(
      "本来の設計思想とマッチしないため、削除される可能性が高いです")]] void
  SetParent(const UUID& parent) {
    parent_ = parent;
  }
  [[deprecated(
      "本来の設計思想とマッチしないため、削除される可能性が高いです")]] void
  SetChildren(const std::vector<UUID>& children) {
    children_ = children;
  }
  [[deprecated(
      "本来の設計思想とマッチしないため、削除される可能性が高いです")]] void
  SetScene(Scene* scene) {
    scene_ = scene;
  }
  Vector2 rotation_vector_;


  static void _Bind();

private:
  Matrix44 local_transform_;
  Vector3 position_ = Vec3::kZero;
  Vector3 scale_ = Vec3::kOne;
  Vector3 rotation_euler_ = Vec3::kZero;
  Quaternion rotation_ = {0.0f, 0.0f, 0.0f, 1.0f};

  Matrix44 global_transform_;
  Vector3 global_position_ = Vec3::kZero;
  Vector3 global_scale_ = Vec3::kOne;
  Vector3 global_rotation_euler_ = Vec3::kZero;
  Quaternion global_rotation_ = {0.0f, 0.0f, 0.0f, 1.0f};

  UUID parent_ = 0;
  std::vector<UUID> children_;

  Scene* scene_;
  mutable bool global_dirty_ = false;
  bool local_dirty_ = false;
  enum TransformDirty {
    DIRTY_NONE = 0,
    DIRTY_EULER_ROTATION_AND_SCALE = 1,
    DIRTY_LOCAL_TRANSFORM = 2,
    DIRTY_GLOBAL_TRANSFORM = 4
  };
  int dirty_ = DIRTY_NONE;
  void UpdateTransform();
  void UpdateGlobalTransform();

  void UpdateFormValues();
  void NotifyTransform() const;

};

}  // namespace base_engine::component
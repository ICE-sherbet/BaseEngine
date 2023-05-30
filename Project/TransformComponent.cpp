#include "TransformComponent.h"

#include "ObjectEntity.h"
#include "Scene.h"

namespace base_engine::component {
TransformComponent::TransformComponent() = default;

TransformComponent::TransformComponent(const Vector3& translation)
    : position_(translation) {}

Matrix44 TransformComponent::GetLocalTransform() const {
  return local_transform_;
}

void TransformComponent::SetLocalTransform(const Matrix44& transform) {
  local_transform_ = transform;
  local_dirty_ = true;
  NotifyTransform();
}

Vector3 TransformComponent::GetLocalRotationEuler() const {
  if (local_dirty_) {
    const_cast<TransformComponent*>(this)->UpdateFormValues();
  }
  return rotation_euler_;
}

void TransformComponent::SetLocalTranslation(const Vector3& pos) {

  position_ = pos;
  UpdateTransform();
}

Vector3 TransformComponent::GetLocalTranslation() const {
  if (local_dirty_)
  {
    const_cast<TransformComponent*>(this)->UpdateFormValues();
  }

  return position_;
}

void TransformComponent::SetLocalScale(const Vector3& scale) {
  scale_ = scale;
  UpdateTransform();
}

Vector3 TransformComponent::GetLocalScale() const {

  return scale_;
}

void TransformComponent::SetLocalRotationEuler(const Vector3& euler) {

  rotation_euler_ = euler;
  rotation_ = Rotation(rotation_euler_);
  UpdateTransform();
}

Quaternion TransformComponent::GetLocalRotation() {
  return rotation_;
}

void TransformComponent::SetGlobalTransform(const Matrix44& transform) {
  const Matrix44 xform =
      (parent_) ? mat::Inverse(scene_->GetEntityWithUUID(parent_)
                                   .GetComponent<TransformComponent>()
                                   .GetGlobalTransform()) *
                      transform
                : transform;
  SetLocalTransform(xform);
}

void TransformComponent::SetGlobalTranslation(const Vector3& pos) {
  if (global_position_ == pos) return;

  global_position_ = pos;
  UpdateGlobalTransform();
  auto transfrom = GetGlobalTransform();
  transfrom.rc[3][0] = pos.x;
  transfrom.rc[3][1] = pos.y;
  transfrom.rc[3][2] = pos.z;
  SetGlobalTransform(transfrom);
}

Vector3 TransformComponent::GetGlobalTranslation() const {
  return {const_cast<MofFloat*>(global_transform_.rc[3])};
}

Vector3 TransformComponent::GetGlobalRotationEuler() const {
  return global_rotation_euler_;
}

Matrix44 TransformComponent::GetGlobalTransform() {
  if (global_dirty_) {
    if (parent_) {
      auto& transform =
          scene_->GetEntityWithUUID(parent_).GetComponent<TransformComponent>();
      global_transform_ = local_transform_ * transform.GetGlobalTransform();
    } else {
      global_transform_ = local_transform_;
    }
  }
  global_dirty_ = false;
  return global_transform_;
}

void TransformComponent::SetLocalRotation(const Quaternion& quat) {
  rotation_ = quat;
  rotation_euler_ = EulerAngles(rotation_);
  UpdateTransform();
}

void TransformComponent::UpdateTransform() {
  const auto s = mat::Scale(Matrix44{}, scale_);
  const auto r = ToMat4(rotation_);
  const auto t = mat::Translate(Matrix44{}, position_);

  local_transform_ = s * r * t;
  NotifyTransform();
}

void TransformComponent::UpdateGlobalTransform() {
  const auto s = mat::Scale(Matrix44{}, global_scale_);
  const auto r = ToMat4(global_rotation_);
  const auto t = mat::Translate(Matrix44{}, global_position_);

  global_transform_ = s * r * t;
  NotifyTransform();
}

void TransformComponent::UpdateFormValues() {
  math::DecomposeTransform(local_transform_, position_, rotation_, scale_);
  rotation_euler_ = EulerAngles(rotation_);
  local_dirty_ = false;
}

void TransformComponent::NotifyTransform() const {
  for (const auto& child : children_) {
    auto& transform =
        scene_->GetEntityWithUUID(child).GetComponent<TransformComponent>();
    transform.NotifyTransform();
  }
  global_dirty_ = true;
}
}  // namespace base_engine::component

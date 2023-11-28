#include "EditorCamera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace base_engine {
EditorCamera::EditorCamera(const float degFov, const float width,
                           const float height, const float nearP,
                           const float farP)
    : Camera(
          glm::perspectiveFov(glm::radians(degFov), width, height, farP, nearP),
          glm::perspectiveFov(glm::radians(degFov), width, height, nearP,
                              farP)),
      focal_point_(0.0f),
      vertical_fov_(glm::radians(degFov)),
      near_clip_(nearP),
      far_clip_(farP) {
  Init();
}

void EditorCamera::Init() {
  constexpr glm::vec3 position = {-5, 5, 5};

  distance_ = glm::distance(position, focal_point_);

  yaw_ = 3.0f * glm::pi<float>() / 4.0f;

  pitch_ = glm::pi<float>() / 4.0f;

  position_ = CalculatePosition();
  const glm::quat orientation = GetOrientation();
  direction_ = glm::eulerAngles(orientation) * (180.0f / glm::pi<float>());
  view_matrix_ =
      glm::translate(glm::mat4(1.0f), position_) * glm::toMat4(orientation);
  view_matrix_ = glm::inverse(view_matrix_);
}

void EditorCamera::Update()
{
  position_ = CalculatePosition();
  UpdateCameraView();
}

float EditorCamera::GetCameraSpeed() const {
  float speed = normal_speed_;
  return glm::clamp(speed, min_speed_, max_speed_);
}

void EditorCamera::UpdateCameraView() {
  const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

  // Extra step to handle the problem when the camera direction is the same as
  // the up vector
  const float cosAngle = glm::dot(GetForwardDirection(), GetUpDirection());
  if (cosAngle * yawSign > 0.99f) pitch_delta_ = 0.f;

  auto f = GetForwardDirection();
  const glm::vec3 lookAt = position_ + GetForwardDirection();
  direction_ = glm::normalize(lookAt - position_);
  distance_ = glm::distance(position_, focal_point_);
  view_matrix_ = glm::lookAt(position_, lookAt, glm::vec3{0.f, yawSign, 0.f});

  // damping for smooth camera
  yaw_delta_ *= 0.6f;
  pitch_delta_ *= 0.6f;
  position_delta_ *= 0.8f;
}

void EditorCamera::Focus(const glm::vec3& focusPoint) {
  focal_point_ = focusPoint;
  camera_mode_ = CameraMode::FLYCAM;
  if (distance_ > min_focus_distance_) {
    distance_ -= distance_ - min_focus_distance_;
    position_ = focal_point_ - GetForwardDirection() * distance_;
  }
  position_ = focal_point_ - GetForwardDirection() * distance_;
  UpdateCameraView();
}

std::pair<float, float> EditorCamera::PanSpeed() const {
  const float x =
      glm::min(float(viewport_width_) / 1000.0f, 2.4f);  // max = 2.4f
  const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

  const float y =
      glm::min(float(viewport_height_) / 1000.0f, 2.4f);  // max = 2.4f
  const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

  return {xFactor, yFactor};
}

float EditorCamera::RotationSpeed() const { return 0.3f; }

float EditorCamera::ZoomSpeed() const {
  float distance = distance_ * 0.2f;
  distance = glm::max(distance, 0.0f);
  float speed = distance * distance;
  speed = glm::min(speed, 50.0f);  // max speed = 50
  return speed;
}

void EditorCamera::MousePan(const glm::vec2& delta) {
  auto [xSpeed, ySpeed] = PanSpeed();
  focal_point_ -= GetRightDirection() * delta.x * xSpeed * distance_;
  focal_point_ += GetUpDirection() * delta.y * ySpeed * distance_;
}

void EditorCamera::MouseRotate(const glm::vec2& delta) {
  const float yawSign = GetUpDirection().y < 0.0f ? -1.0f : 1.0f;
  yaw_delta_ += yawSign * delta.x * RotationSpeed();
  pitch_delta_ += delta.y * RotationSpeed();
}

void EditorCamera::MouseZoom(float delta) {
  distance_ -= delta * ZoomSpeed();
  const glm::vec3 forwardDir = GetForwardDirection();
  position_ = focal_point_ - forwardDir * distance_;
  if (distance_ < 1.0f) {
    focal_point_ += forwardDir * distance_;
    distance_ = 1.0f;
  }
  position_delta_ += delta * ZoomSpeed() * forwardDir;
}

glm::vec3 EditorCamera::GetUpDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 EditorCamera::GetRightDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
}

glm::vec3 EditorCamera::GetForwardDirection() const {
  return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 EditorCamera::CalculatePosition() const {
  return focal_point_ - GetForwardDirection() * distance_ + position_delta_;
}

glm::quat EditorCamera::GetOrientation() const {
  return glm::quat(
      glm::vec3(-pitch_ - pitch_delta_, -yaw_ - yaw_delta_, 0.0f));
}
}  // namespace base_engine
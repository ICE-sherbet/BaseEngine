// @EditorCamera.h
// @brief
// @author ICE
// @date 2023/11/28
//
// @details

#pragma once
#include "Camera.h"

namespace base_engine {
enum class CameraMode { NONE, FLYCAM, ARCBALL };

class EditorCamera : public Camera {
 public:
  EditorCamera(const float degFov, const float width, const float height,
               const float nearP, const float farP);
  void Init();

  void Update();

  void Focus(const glm::vec3& focusPoint);

  bool IsActive() const { return is_active_; }
  void SetActive(bool active) { is_active_ = active; }

  CameraMode GetCurrentMode() const { return camera_mode_; }

  inline float GetDistance() const { return distance_; }
  inline void SetDistance(float distance) { distance_ = distance; }

  const glm::vec3& GetFocalPoint() const { return focal_point_; }

  inline void SetViewportSize(uint32_t width, uint32_t height) {
    if (viewport_width_ == width && viewport_height_ == height) return;
    SetPerspectiveProjectionMatrix(vertical_fov_, (float)width, (float)height,
                                   near_clip_, far_clip_);
    viewport_width_ = width;
    viewport_height_ = height;
  }

  const glm::mat4& GetViewMatrix() const { return view_matrix_; }
  glm::mat4 GetViewProjection() const {
    return GetProjectionMatrix() * view_matrix_;
  }
  glm::mat4 GetUnReversedViewProjection() const {
    return GetUnReversedProjectionMatrix() * view_matrix_;
  }

  glm::vec3 GetUpDirection() const;
  glm::vec3 GetRightDirection() const;
  glm::vec3 GetForwardDirection() const;

  const glm::vec3& GetPosition() const { return position_; }

  glm::quat GetOrientation() const;

  [[nodiscard]] float GetVerticalFOV() const { return vertical_fov_; }
  [[nodiscard]] float GetAspectRatio() const { return aspect_ratio_; }
  [[nodiscard]] float GetNearClip() const { return near_clip_; }
  [[nodiscard]] float GetFarClip() const { return far_clip_; }
  [[nodiscard]] float GetPitch() const { return pitch_; }
  [[nodiscard]] float GetYaw() const { return yaw_; }
  [[nodiscard]] float GetCameraSpeed() const;

 private:
  void UpdateCameraView();

  void MousePan(const glm::vec2& delta);
  void MouseRotate(const glm::vec2& delta);
  void MouseZoom(float delta);

  glm::vec3 CalculatePosition() const;

  std::pair<float, float> PanSpeed() const;
  float RotationSpeed() const;
  float ZoomSpeed() const;

 private:
  glm::mat4 view_matrix_;
  glm::vec3 position_, direction_, focal_point_;

  float vertical_fov_, aspect_ratio_, near_clip_, far_clip_;

  bool is_active_ = false;
  bool panning_, rotating_;
  glm::vec2 initial_mouse_position_{};
  glm::vec3 initial_focal_point_, initial_rotation_;

  float distance_;
  float normal_speed_{0.002f};

  float pitch_, yaw_;
  float pitch_delta_{}, yaw_delta_{};
  glm::vec3 position_delta_{};
  glm::vec3 right_direction_{};

  CameraMode camera_mode_{CameraMode::ARCBALL};

  float min_focus_distance_{100.0f};

  uint32_t viewport_width_{1280}, viewport_height_{720};

  constexpr static float min_speed_{0.0005f}, max_speed_{2.0f};
  friend class EditorLayer;
};
}  // namespace base_engine

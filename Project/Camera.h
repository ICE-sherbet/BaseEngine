// @Camera.h
// @brief
// @author ICE
// @date 2023/11/28
//
// @details

#pragma once
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>

namespace base_engine {

class Camera {
 public:
  Camera() = default;
  Camera(const glm::mat4& projection, const glm::mat4& unReversedProjection);
  Camera(const float degFov, const float width, const float height,
         const float nearP, const float farP);
  virtual ~Camera() = default;

  const glm::mat4& GetProjectionMatrix() const { return projection_matrix_; }
  const glm::mat4& GetUnReversedProjectionMatrix() const {
    return un_reversed_projection_matrix_;
  }

  void SetProjectionMatrix(const glm::mat4 projection,
                           const glm::mat4 unReversedProjection) {
    projection_matrix_ = projection;
    un_reversed_projection_matrix_ = unReversedProjection;
  }

  void SetPerspectiveProjectionMatrix(const float radFov, const float width,
                                      const float height, const float nearP,
                                      const float farP) {
    projection_matrix_ =
        glm::perspectiveFov(radFov, width, height, farP, nearP);
    un_reversed_projection_matrix_ =
        glm::perspectiveFov(radFov, width, height, nearP, farP);
  }

  void SetOrthoProjectionMatrix(const float width, const float height,
                                const float nearP, const float farP) {
    projection_matrix_ = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f,
                                    height * 0.5f, farP, nearP);
    un_reversed_projection_matrix_ =
        glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f,
                   nearP, farP);
  }

  float GetExposure() const { return exposure_; }
  float& GetExposure() { return exposure_; }

 protected:
  float exposure_ = 0.8f;

 private:
  glm::mat4 projection_matrix_ = glm::mat4(1.0f);
  glm::mat4 un_reversed_projection_matrix_ = glm::mat4(1.0f);
};

}  // namespace base_engine

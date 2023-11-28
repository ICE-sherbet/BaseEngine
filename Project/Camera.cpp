#include "Camera.h"

namespace base_engine {
Camera::Camera(const glm::mat4& projection,
               const glm::mat4& unReversedProjection)
    : projection_matrix_(projection),
      un_reversed_projection_matrix_(unReversedProjection) {}

Camera::Camera(const float degFov, const float width, const float height,
               const float nearP, const float farP)
    : projection_matrix_(glm::perspectiveFov(glm::radians(degFov), width,
                                             height, farP, nearP)),
      un_reversed_projection_matrix_(glm::perspectiveFov(
          glm::radians(degFov), width, height, nearP, farP)) {}
}  // namespace base_engine
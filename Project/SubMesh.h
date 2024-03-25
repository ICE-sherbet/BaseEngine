// @SubMesh.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include <glm/glm.hpp>
#include <string>

#include "AABB.h"

namespace base_engine {

class SubMesh {
 public:
  uint32_t BaseVertex;
  uint32_t BaseIndex;
  uint32_t MaterialIndex;
  uint32_t IndexCount;
  uint32_t VertexCount;

  glm::mat4 Transform{1.0f};
  glm::mat4 LocalTransform{1.0f};
  AABB BoundingBox;

  std::string NodeName, MeshName;
  bool IsRigged = false;
};
}  // namespace base_engine
